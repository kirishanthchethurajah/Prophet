#include "ProphetRoutingLayer.h"

Define_Module(ProphetRoutingLayer);

void ProphetRoutingLayer::initialize(int stage)
{
    if (stage == 0) {
        // get parameters
        ownMACAddress = par("ownMACAddress").stringValue();
        nextAppID = 1;
        maximumCacheSize = par("maximumCacheSize");
        currentCacheSize = 0;
        antiEntropyInterval = par("antiEntropyInterval");
        maximumHopCount = par("maximumHopCount");
        maximumRandomBackoffDuration = par("maximumRandomBackoffDuration");
        logging = par("logging");
        useTTL = par("useTTL");
        numEventsHandled = 0;

        syncedNeighbourListIHasChanged = TRUE;

        //Parameters for Prophet Protocol
        pEncounterMax = par("pEncounterMax");
        pEncounterFirst = par("pEncounterFirst");
	pFirstThreshold = par("pFirstThreshold");
	alpha = par("alpha");
	beta = par("beta");
	gamma = par("gamma");
	delta = par("delta");
	lastTimeAged = 0.0;
        standardTimeInterval = par("standardTimeInterval");

	} else if (stage == 1) {
    } else if (stage == 2) {
    } else {
        EV_FATAL << PROPHETROUTINGLAYER_SIMMODULEINFO << "Something is radically wrong in initialisation \n";
    }
}

int ProphetRoutingLayer::numInitStages() const
{
    return 3;
}

void ProphetRoutingLayer::handleMessage(cMessage *msg)
{
    cGate *gate;
    char gateName[64];
    KNeighbourListMsg *neighListMsg;

    numEventsHandled++;

	  // age the data in the cache only if needed (e.g. a message arrived)
    if (useTTL)
    	ageDataInCache();

    // self messages
    if (msg->isSelfMessage()) {
	EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << "Received unexpected self message" << "\n";
	delete msg;
    // messages from other layers
    } else {

	// get message arrival gate name
        gate = msg->getArrivalGate();
        strcpy(gateName, gate->getName());

        // app registration message arrived from the upper layer (app layer)
        if (strstr(gateName, "upperLayerIn") != NULL && dynamic_cast<KRegisterAppMsg*>(msg) != NULL) {

	    handleAppRegistrationMsg(msg);
        // data message arrived from the upper layer (app layer)
        } else if (strstr(gateName, "upperLayerIn") != NULL && dynamic_cast<KDataMsg*>(msg) != NULL) {

            handleDataMsgFromUpperLayer(msg);
        // feedback message arrived from the upper layer (app layer)
        } else if (strstr(gateName, "upperLayerIn") != NULL && dynamic_cast<KFeedbackMsg*>(msg) != NULL) {

        // with Prophet Routing, no feedback is considered
            delete msg;
        // neighbour list message arrived from the lower layer (link layer)
        } else if (strstr(gateName, "lowerLayerIn") != NULL && dynamic_cast<KNeighbourListMsg*>(msg) != NULL) {

            handleNeighbourListMsgFromLowerLayer(msg);
        // data message arrived from the lower layer (link layer)
        } else if (strstr(gateName, "lowerLayerIn") != NULL && dynamic_cast<KDataMsg*>(msg) != NULL) {

            handleDataMsgFromLowerLayer(msg);
        // feedback message arrived from the lower layer (link layer)
        } else if (strstr(gateName, "lowerLayerIn") != NULL && dynamic_cast<KFeedbackMsg*>(msg) != NULL) {

        // with Prophet Routing, no feedback is considered
            delete msg;
        // DP table request arrived from the lower layer (link layer)
        } else if (strstr(gateName, "lowerLayerIn") != NULL && dynamic_cast<DPtableRequestMsg*>(msg) != NULL) {

            handleDPTableRequestFromLowerLayer(msg);
        // DP table received from the lower layer (link layer)
        }  else if (strstr(gateName, "lowerLayerIn") != NULL && dynamic_cast<DPtableDataMsg*>(msg) != NULL) {

            handleDPTableDataFromLowerLayer(msg);
        } else {

            EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << "Received unexpected packet" << "\n";
            delete msg;
        }
    }
}

void ProphetRoutingLayer::ageDataInCache()
{

    // remove expired data items
    int originalSize = cacheList.size();
    int expiredFound = TRUE;
    while (expiredFound) {
        expiredFound = FALSE;

        CacheEntry *cacheEntry;
        list<CacheEntry*>::iterator iteratorCache;
        iteratorCache = cacheList.begin();
        while (iteratorCache != cacheList.end()) {
            cacheEntry = *iteratorCache;
            if (cacheEntry->validUntilTime < simTime().dbl()) {
                expiredFound = TRUE;
                break;
            }
            iteratorCache++;
        }
        if (expiredFound) {
            currentCacheSize -= cacheEntry->realPacketSize;
            cacheList.remove(cacheEntry);
            delete cacheEntry;

        }
    }

}

int ProphetRoutingLayer::agingDP()
{
    // Calculate DP values due to Aging

    double currentTime = simTime().dbl() ;
    int count = 0;
    DeliveryPredictability *deliveryPredictability;
    list<DeliveryPredictability*>::iterator iteratorDeliveryPredictability;
    iteratorDeliveryPredictability = dpList.begin();
	
    if(currentTime < lastTimeAged)
    {
	cout<<"Error: something went wrong \n";
	exit(1);
    }
	
    double kappa = (currentTime - lastTimeAged) / standardTimeInterval ;
            
    while (iteratorDeliveryPredictability != dpList.end()) {
        deliveryPredictability = *iteratorDeliveryPredictability;
	if(deliveryPredictability->nodeMACAddress != ownMACAddress){      
            double dP = deliveryPredictability->nodeDP;
	    dP = dP * pow(gamma, kappa);
	    deliveryPredictability->nodeDP = dP;
	    if(dP >=  pFirstThreshold){
	    	count ++;
	    }
        } 
        iteratorDeliveryPredictability++;
    }
    lastTimeAged =  currentTime;
    return count;
}


void ProphetRoutingLayer::handleAppRegistrationMsg(cMessage *msg)
{
    KRegisterAppMsg *regAppMsg = dynamic_cast<KRegisterAppMsg*>(msg);
    AppInfo *appInfo = NULL;
    int found = FALSE;
    list<AppInfo*>::iterator iteratorRegisteredApps = registeredAppList.begin();
    while (iteratorRegisteredApps != registeredAppList.end()) {
        appInfo = *iteratorRegisteredApps;
        if (appInfo->appName == regAppMsg->getAppName()) {
            found = TRUE;
            break;
        }
        iteratorRegisteredApps++;
    }

    if (!found) {
        appInfo = new AppInfo;
        appInfo->appID = nextAppID++;
        appInfo->appName = regAppMsg->getAppName();
        registeredAppList.push_back(appInfo);

    }
    appInfo->prefixName = regAppMsg->getPrefixName();
    delete msg;

}

void ProphetRoutingLayer::handleDataMsgFromUpperLayer(cMessage *msg)
{
    KDataMsg *omnetDataMsg = dynamic_cast<KDataMsg*>(msg);

    if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<UI>!<DM>!<" << omnetDataMsg->getSourceAddress() << ">!<"
        << omnetDataMsg->getDestinationAddress() << ">!<" << omnetDataMsg->getFinalDestinationNodeAddress() << ">!<" << omnetDataMsg->getDataName() << ">!<" << 	omnetDataMsg->getGoodnessValue() << ">!<" << omnetDataMsg->getByteLength() << ">!<" << omnetDataMsg->getHopsTravelled() << "\n";}


    CacheEntry *cacheEntry;
    list<CacheEntry*>::iterator iteratorCache;
    int found = FALSE;
    iteratorCache = cacheList.begin();
    while (iteratorCache != cacheList.end()) {
        cacheEntry = *iteratorCache;
        //Restrict non -destination oriented message as well as the duplicate message
        if (cacheEntry->dataName == omnetDataMsg->getDataName()){ 
		    found = TRUE;
            break;
        }

        iteratorCache++;
    }

    if (!found) {
        // apply caching policy if limited cache and cache is full
        if (maximumCacheSize != 0
                && (currentCacheSize + omnetDataMsg->getRealPayloadSize()) > maximumCacheSize
                && cacheList.size() > 0) {
            iteratorCache = cacheList.begin();
            CacheEntry *removingCacheEntry = *iteratorCache;
            iteratorCache = cacheList.begin();
            while (iteratorCache != cacheList.end()) {
                cacheEntry = *iteratorCache;
                if (cacheEntry->validUntilTime < removingCacheEntry->validUntilTime) {
                    removingCacheEntry = cacheEntry;
                }
                iteratorCache++;
            }
            currentCacheSize -= removingCacheEntry->realPayloadSize;
            cacheList.remove(removingCacheEntry);

			//if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<CR>!<"
                //<< removingCacheEntry->dataName << ">!<" << removingCacheEntry->realPayloadSize << ">!<0>!<"
                //<< currentCacheSize << ">!<0>!<0>!<" << removingCacheEntry->hopsTravelled << "\n";}

            delete removingCacheEntry;

        }

        cacheEntry = new CacheEntry;

        cacheEntry->messageID = omnetDataMsg->getDataName();
        cacheEntry->hopCount = 0;
        cacheEntry->dataName = omnetDataMsg->getDataName();
        cacheEntry->realPayloadSize = omnetDataMsg->getRealPayloadSize();
        cacheEntry->dummyPayloadContent = omnetDataMsg->getDummyPayloadContent();
        cacheEntry->validUntilTime = omnetDataMsg->getValidUntilTime();
        cacheEntry->realPacketSize = omnetDataMsg->getRealPacketSize();
        cacheEntry->originatorNodeName = omnetDataMsg->getOriginatorNodeName();
        cacheEntry->destinationOriented = omnetDataMsg->getDestinationOriented();
        if (omnetDataMsg->getDestinationOriented()) {
            cacheEntry->finalDestinationNodeName = omnetDataMsg->getFinalDestinationNodeName();
            cacheEntry->finalDestinationNodeAddress = omnetDataMsg->getFinalDestinationNodeAddress();
        }
        cacheEntry->goodnessValue = omnetDataMsg->getGoodnessValue();
        cacheEntry->hopsTravelled = 0;
        cacheEntry->createdTime = simTime().dbl();
        cacheEntry->updatedTime = simTime().dbl();

        cacheList.push_back(cacheEntry);

        currentCacheSize += cacheEntry->realPayloadSize;

    }

    cacheEntry->lastAccessedTime = simTime().dbl();

	
    delete msg;
}

void ProphetRoutingLayer::handleNeighbourListMsgFromLowerLayer(cMessage *msg)
{
    KNeighbourListMsg *neighListMsg = dynamic_cast<KNeighbourListMsg*>(msg);

    //if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<NM>!<NC>!<" <<
			//			neighListMsg->getNeighbourNameListArraySize() << ">!<CS>!<"
        //                    << cacheList.size() << "\n";}

     // if no neighbours or cache is empty, just return
    if (neighListMsg->getNeighbourNameListArraySize() == 0 || cacheList.size() == 0) {

        // setup sync neighbour list for the next time - only if there were some changes
        if (syncedNeighbourListIHasChanged) {
            setSyncingNeighbourInfoForNoNeighboursOrEmptyCache();
            syncedNeighbourListIHasChanged = FALSE;
        }

        delete msg;
        return;
     }

     // send DPtable request messages (if appropriate) to all nodes
     int i = 0;
     while (i < neighListMsg->getNeighbourNameListArraySize()) {
        string nodeMACAddress = neighListMsg->getNeighbourNameList(i);
        SyncedNeighbour *syncedNeighbour = getSyncingNeighbourInfo(nodeMACAddress);

        // indicate that this node was considered this time
        syncedNeighbour->nodeConsidered = TRUE;

        bool syncWithNeighbour = FALSE;

        if (syncedNeighbour->syncCoolOffEndTime >= simTime().dbl()) {
            // if the sync was done recently, don't sync again until the anti-entropy interval
            // has elapsed
            syncWithNeighbour = FALSE;

        } else if (syncedNeighbour->randomBackoffStarted && syncedNeighbour->randomBackoffEndTime >= simTime().dbl()) {
            // if random backoff to sync is still active, then wait until time expires
            syncWithNeighbour = FALSE;

        }else if (syncedNeighbour->neighbourSyncing && syncedNeighbour->neighbourSyncEndTime >= simTime().dbl()) {
            // if this neighbour has started syncing with me, then wait until this neighbour finishes
            syncWithNeighbour = FALSE;

        } else if (syncedNeighbour->randomBackoffStarted && syncedNeighbour->randomBackoffEndTime < simTime().dbl()) {
            // has the random backoff just finished - if so, then my turn to start the syncing process
            syncWithNeighbour = TRUE;


        } else if (syncedNeighbour->neighbourSyncing && syncedNeighbour->neighbourSyncEndTime < simTime().dbl()) {
            // has the neighbours syncing period elapsed - if so, my turn to sync
            syncWithNeighbour = TRUE;

        } else {
            // neighbour seen for the first time (could also be after the cool off period)
            // then start the random backoff
            syncedNeighbour->randomBackoffStarted = TRUE;
            double randomBackoffDuration = uniform(1.0, maximumRandomBackoffDuration);
            syncedNeighbour->randomBackoffEndTime = simTime().dbl() + randomBackoffDuration;
            syncWithNeighbour = FALSE;
        }

        // from previous questions - if syncing required
        if (syncWithNeighbour) {
            // set the cooloff period
            syncedNeighbour->syncCoolOffEndTime = simTime().dbl() + antiEntropyInterval;
            syncedNeighbour->randomBackoffStarted = FALSE;
            syncedNeighbour->randomBackoffEndTime = 0.0;
            syncedNeighbour->neighbourSyncing = FALSE;
            syncedNeighbour->neighbourSyncEndTime = 0.0;
            //Calculate Encounter Interval for DPT calculation
            if(syncedNeighbour->lastSyncTime != 0.0){
		int numPreviousEncounter = syncedNeighbour->numPreviousEncounter;
		double totalAEI = syncedNeighbour->totalAEI;
		double aEIValue = syncedNeighbour->aEIValue;
		double lastSyncTime = syncedNeighbour->lastSyncTime;
		numPreviousEncounter ++;
                //History of Encounter check
		totalAEI += simTime().dbl() - lastSyncTime;
		aEIValue = totalAEI / numPreviousEncounter;
		// Update the existing value
		syncedNeighbour->numPreviousEncounter = numPreviousEncounter;

		syncedNeighbour->totalAEI = totalAEI ;
		syncedNeighbour->aEIValue = aEIValue  ;

		}
				
		
            // send DPtable request message
            DPtableRequestMsg *dptableRequestMsg = new DPtableRequestMsg();
	dptableRequestMsg->setSourceAddress(ownMACAddress.c_str());
	dptableRequestMsg->setDestinationAddress(nodeMACAddress.c_str());
	send(dptableRequestMsg, "lowerLayerOut");

	if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LO>!<DPR>!<"
		  << dptableRequestMsg->getDestinationAddress()<< "\n";}

        }

        i++;
    }

   // setup sync neighbour list for the next time
   setSyncingNeighbourInfoForNextRound();

   // synched neighbour list must be updated in next round
   // as there were changes
   syncedNeighbourListIHasChanged = TRUE;

   // delete the received neighbor list msg
   delete msg;

}


ProphetRoutingLayer::SyncedNeighbour* ProphetRoutingLayer::getSyncingNeighbourInfo(string nodeMACAddress)
{
    // check if sync entry is there
    SyncedNeighbour *syncedNeighbour = NULL;
    list<SyncedNeighbour*>::iterator iteratorSyncedNeighbour;
    bool found = FALSE;
    iteratorSyncedNeighbour = syncedNeighbourList.begin();
    while (iteratorSyncedNeighbour != syncedNeighbourList.end()) {
        syncedNeighbour = *iteratorSyncedNeighbour;
        if (syncedNeighbour->nodeMACAddress == nodeMACAddress) {

            found = TRUE;
            break;
        }

        iteratorSyncedNeighbour++;
    }

    if (!found) {

    //Create an Entry in DPT table
	updateNeighbourSyncStarted(nodeMACAddress,ownMACAddress);
	// if sync entry not there, create an entry with initial values
    syncedNeighbour = new SyncedNeighbour;
	syncedNeighbour->nodeMACAddress = nodeMACAddress.c_str();
    syncedNeighbour->syncCoolOffEndTime = 0.0;
    syncedNeighbour->randomBackoffStarted = FALSE;
    syncedNeighbour->randomBackoffEndTime = 0.0;
    syncedNeighbour->neighbourSyncing = FALSE;
    syncedNeighbour->neighbourSyncEndTime = 0.0;
    syncedNeighbour->nodeConsidered = FALSE;
	syncedNeighbour->numPreviousEncounter = 0;
	syncedNeighbour->totalAEI = 0.0;
	syncedNeighbour->aEIValue = 0.0;
	syncedNeighbour->lastSyncTime = 0.0;
	syncedNeighbourList.push_back(syncedNeighbour);
    }

    return syncedNeighbour;
}

void ProphetRoutingLayer::handleDPTableRequestFromLowerLayer(cMessage *msg)
{

	DPtableRequestMsg *dptableRequestMsg = dynamic_cast<DPtableRequestMsg*>(msg);



	//if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LI>!<DPRM>!<" << dptableRequestMsg->getSourceAddress() << ">!<"
      //          << dptableRequestMsg->getByteLength() << "\n";}

	//cout<< " Entering handleDPTableRequestFromLowerLayer " <<"\n";
	//cout<< "Dp list size :" << dpList.size()<<"\n";
	int listSize = agingDP();
    //cout<< "sending Dp list size :" << listSize <<"\n";
	DPtableDataMsg *dptableDataMsg = new DPtableDataMsg();
	dptableDataMsg->setDpListArraySize(listSize);
    list<DeliveryPredictability*>::iterator iteratorDeliveryPredictability;
    iteratorDeliveryPredictability = dpList.begin();
    int i=0;
	while (iteratorDeliveryPredictability != dpList.end()) {
  		DeliveryPredictability *deliveryPredictability = *iteratorDeliveryPredictability;
	    // Send only Dp values greater than the first threashold value to reduce the data load
    	if(deliveryPredictability->nodeDP >=  pFirstThreshold)
	    {
			if (i < listSize)
			    {
        	    delivPredictability temp;
			    temp.nodeMACAddress= deliveryPredictability->nodeMACAddress;
			    temp.nodeDP = deliveryPredictability->nodeDP;
				dptableDataMsg->setDpList(i,temp);
		        i++;
                }		
		}
		iteratorDeliveryPredictability++;

  	}

	dptableDataMsg->setSourceAddress(dptableRequestMsg->getDestinationAddress());
	dptableDataMsg->setDestinationAddress(dptableRequestMsg->getSourceAddress());

	if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LO>!<DPTM>!<" << dptableRequestMsg->getSourceAddress() << ">!<"
                << dpList.size() << ">!<" << listSize << "\n";}
	send(dptableDataMsg, "lowerLayerOut");
	delete msg;

}

void ProphetRoutingLayer::handleDPTableDataFromLowerLayer(cMessage *msg)
{
    DPtableDataMsg *dptableDataMsg  = dynamic_cast<DPtableDataMsg*>(msg);
    int i= 0;
    //cout<<"Entering Handle Dpt table data from lower layer :\n";
    //cout <<"DP list size: "<< dptableDataMsg->getDpListArraySize() <<"\n";
    while (i< dptableDataMsg->getDpListArraySize())
    {
  	    delivPredictability temp;
        DeliveryPredictability *deliveryPredictability = new DeliveryPredictability;
        temp =  dptableDataMsg->getDpList(i);
      	deliveryPredictability->nodeMACAddress = temp.nodeMACAddress.c_str();
      	deliveryPredictability->nodeDP = temp.nodeDP ;
	      dpListReceived.push_back(deliveryPredictability);
	      i++;
    }

	//if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LI>!<DPTM>!<" << dptableDataMsg->getSourceAddress() << ">!<"
      //          << dptableDataMsg->getDpListArraySize() << ">!<" << "\n";}

    updateDPT(dptableDataMsg->getSourceAddress());

	  delete msg;
}

void ProphetRoutingLayer::sendDataMsg(vector<string> destinationNodes, string nodeMACAddress)
{
    
     CacheEntry *cacheEntry;
    list<CacheEntry*>::iterator iteratorCache;
    vector<string> sentdatanamelist;
    iteratorCache = cacheList.begin();
    while (iteratorCache != cacheList.end()) {
        cacheEntry = *iteratorCache;
        if (std::find(destinationNodes.begin(), destinationNodes.end(), cacheEntry->finalDestinationNodeAddress)!=destinationNodes.end())
  	    {
	        KDataMsg *dataMsg = new KDataMsg();

		dataMsg->setSourceAddress(ownMACAddress.c_str());
	 	dataMsg->setDestinationAddress(nodeMACAddress.c_str());
		dataMsg->setDataName(cacheEntry->dataName.c_str());
		dataMsg->setDummyPayloadContent(cacheEntry->dummyPayloadContent.c_str());
		dataMsg->setValidUntilTime(cacheEntry->validUntilTime);
		dataMsg->setRealPayloadSize(cacheEntry->realPayloadSize);
		// check KOPSMsg.msg on sizing mssages
		int realPacketSize = 6 + 6 + 2 + cacheEntry->realPayloadSize + 4 + 6 + 1;
		dataMsg->setRealPacketSize(realPacketSize);
		dataMsg->setByteLength(realPacketSize);
		dataMsg->setOriginatorNodeName(cacheEntry->originatorNodeName.c_str());
		dataMsg->setDestinationOriented(cacheEntry->destinationOriented);
		if (cacheEntry->destinationOriented) {
		    dataMsg->setFinalDestinationNodeName(cacheEntry->finalDestinationNodeName.c_str());
		    dataMsg->setFinalDestinationNodeAddress(cacheEntry->finalDestinationNodeAddress.c_str());
		    }
		dataMsg->setMessageID(cacheEntry->messageID.c_str());
		dataMsg->setHopCount(cacheEntry->hopCount);
		dataMsg->setGoodnessValue(cacheEntry->goodnessValue);
		dataMsg->setHopsTravelled(cacheEntry->hopsTravelled);

		if(cacheEntry->finalDestinationNodeAddress.c_str() == nodeMACAddress)
                {
                    sentdatanamelist.push_back(cacheEntry->dataName.c_str());
                }

                    //cout<<"data message is about to send\n";
          	send(dataMsg, "lowerLayerOut");

		if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LO>!<DM>!<" << 
           			  dataMsg->getFinalDestinationNodeAddress() << ">!<"<< dataMsg->getDestinationAddress() << ">!<" 
                                  << dataMsg->getByteLength() << ">!<" << dataMsg->getDataName() << ">!<" << dataMsg->getHopsTravelled() << "\n";}
  	    } 
	    iteratorCache++;
    }    
       
   
// remove  data items that are sent to the final destination
   bool expiredFound = FALSE;
   for(int i = 0; i < sentdatanamelist.size(); i++) {
      expiredFound = FALSE;
      CacheEntry *cacheEntry;
      list<CacheEntry*>::iterator iteratorCache;
      iteratorCache = cacheList.begin();
      while (iteratorCache != cacheList.end()) {
          cacheEntry = *iteratorCache;
          if (cacheEntry->dataName.c_str() == sentdatanamelist[i]) {
              expiredFound = TRUE;
              break;
          }
          iteratorCache++;
      }
      if (expiredFound) {
          currentCacheSize -= cacheEntry->realPayloadSize;
        cout<<cacheEntry->dataName.c_str()<<"\n";
        cacheList.remove(cacheEntry);

        delete cacheEntry;
      }
    }
  

	//Calculate the last sync time
    disconnectEncounterInterval(nodeMACAddress);


}


void ProphetRoutingLayer::updateNeighbourSyncStarted(string nodeMACAddress , string ownMACAddress )
{

    // Iniitialisation of  A DPT table
    DeliveryPredictability *deliveryPredictability = new DeliveryPredictability;

    if(nodeMACAddress== ownMACAddress)
    {
        deliveryPredictability->nodeDP = 1.0;
	deliveryPredictability->nodeMACAddress = ownMACAddress;
	
    }else{
        deliveryPredictability->nodeDP = 0.0;
	deliveryPredictability->nodeMACAddress = nodeMACAddress.c_str();
    }


    dpList.push_back(deliveryPredictability);

	
}



void ProphetRoutingLayer::handleDataMsgFromLowerLayer(cMessage *msg)
{
    KDataMsg *omnetDataMsg = dynamic_cast<KDataMsg*>(msg);
    bool found;
    // increment the travelled hop count
    omnetDataMsg->setHopsTravelled(omnetDataMsg->getHopsTravelled() + 1);
    omnetDataMsg->setHopCount(omnetDataMsg->getHopCount() + 1);
    if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<LI>!<DM>!<" << omnetDataMsg->getSourceAddress() << ">!<"
        << omnetDataMsg->getFinalDestinationNodeAddress() << ">!<" << omnetDataMsg->getDataName() << ">!<" << omnetDataMsg->getGoodnessValue() << ">!<"
        << omnetDataMsg->getByteLength() << ">!<" << omnetDataMsg->getHopsTravelled() << "\n";}

    // if destination oriented data sent around and this node is the destination
    // or if maximum hop count is reached
    // then cache or else don't cache
    bool cacheData = TRUE;
    if ((omnetDataMsg->getDestinationOriented()
         && strstr(ownMACAddress.c_str(), omnetDataMsg->getFinalDestinationNodeAddress()) != NULL)
         || omnetDataMsg->getHopCount() >= maximumHopCount) {
     //Messages reached its destination or should be delted as it reaches maximum hop count
        cacheData = FALSE;
    }

    if(cacheData) {

        // insert/update cache
        CacheEntry *cacheEntry;
        list<CacheEntry*>::iterator iteratorCache;
        found = FALSE;
        iteratorCache = cacheList.begin();
        while (iteratorCache != cacheList.end()) {
            cacheEntry = *iteratorCache;
            if (cacheEntry->dataName == omnetDataMsg->getDataName() || (!omnetDataMsg->getDestinationOriented()) ) {
                found = TRUE;
                break;
            }


            iteratorCache++;
        }

        if (!found) {

            // apply caching policy if limited cache and cache is full
            if (maximumCacheSize != 0
                && (currentCacheSize + omnetDataMsg->getRealPayloadSize()) > maximumCacheSize
                && cacheList.size() > 0) {
                iteratorCache = cacheList.begin();
                advance(iteratorCache, 0);
                CacheEntry *removingCacheEntry = *iteratorCache;
                iteratorCache = cacheList.begin();
                while (iteratorCache != cacheList.end()) {
                    cacheEntry = *iteratorCache;
                    if (cacheEntry->validUntilTime < removingCacheEntry->validUntilTime) {
                        removingCacheEntry = cacheEntry;
                    }
                    iteratorCache++;
                }
                currentCacheSize -= removingCacheEntry->realPayloadSize;
                cacheList.remove(removingCacheEntry);

				if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<CR>!<"
                    << removingCacheEntry->dataName << ">!<" << removingCacheEntry->realPayloadSize << ">!<0>!<"
                    << currentCacheSize << ">!<0>!<0>!<" << removingCacheEntry->hopsTravelled << "\n";}
                delete removingCacheEntry;

            }

            cacheEntry = new CacheEntry;

            cacheEntry->messageID = omnetDataMsg->getMessageID();
            cacheEntry->dataName = omnetDataMsg->getDataName();
            cacheEntry->realPayloadSize = omnetDataMsg->getRealPayloadSize();
            cacheEntry->dummyPayloadContent = omnetDataMsg->getDummyPayloadContent();
            cacheEntry->validUntilTime = omnetDataMsg->getValidUntilTime();
            cacheEntry->realPacketSize = omnetDataMsg->getRealPacketSize();
            cacheEntry->originatorNodeName = omnetDataMsg->getOriginatorNodeName();
            cacheEntry->destinationOriented = omnetDataMsg->getDestinationOriented();
            if (omnetDataMsg->getDestinationOriented()) {
                cacheEntry->finalDestinationNodeName = omnetDataMsg->getFinalDestinationNodeName();
                cacheEntry->finalDestinationNodeAddress = omnetDataMsg->getFinalDestinationNodeAddress();
            }
			cacheEntry->goodnessValue = omnetDataMsg->getGoodnessValue();
            cacheEntry->createdTime = simTime().dbl();
            cacheEntry->updatedTime = simTime().dbl();

            cacheList.push_back(cacheEntry);

            currentCacheSize += cacheEntry->realPayloadSize;

        }
		cacheEntry->hopsTravelled = omnetDataMsg->getHopsTravelled();
        cacheEntry->hopCount = omnetDataMsg->getHopCount() ;
        cacheEntry->lastAccessedTime = simTime().dbl();



		// log cache update or add
        if (found) {
            if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<CU>!<"
                << omnetDataMsg->getDataName() << ">!<" << omnetDataMsg->getRealPayloadSize() << ">!<0>!<"
                << currentCacheSize << ">!<0>!<0>!<" << omnetDataMsg->getHopsTravelled() << "\n";}
        } else {
            if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<CA>!<"
                << omnetDataMsg->getDataName() << ">!<" << omnetDataMsg->getRealPayloadSize() << ">!<0>!<"
                << currentCacheSize << ">!<0>!<0>!<" << omnetDataMsg->getHopsTravelled() << "\n";}
        }
    }

    // if registered app exist, send data msg to app
    AppInfo *appInfo = NULL;
    found = FALSE;
    list<AppInfo*>::iterator iteratorRegisteredApps = registeredAppList.begin();
    while (iteratorRegisteredApps != registeredAppList.end()) {
        appInfo = *iteratorRegisteredApps;
        if (strstr(omnetDataMsg->getDataName(), appInfo->prefixName.c_str()) != NULL) {
            found = TRUE;
            break;
        }
        iteratorRegisteredApps++;
    }
    if (found) {
		//<<"sending the msg to its destination app layer:\n";
        send(msg, "upperLayerOut");


	if (logging) {EV_INFO << PROPHETROUTINGLAYER_SIMMODULEINFO << ">!<" << ownMACAddress << ">!<UO>!<DM>!<" << omnetDataMsg->getSourceAddress() << ">!<"
            << omnetDataMsg->getDestinationAddress() << ">!<" << omnetDataMsg->getDataName() << ">!<" << omnetDataMsg->getGoodnessValue() << ">!<"
            << omnetDataMsg->getByteLength() << ">!<" << omnetDataMsg->getHopsTravelled() << "\n";}


    } else {
        delete msg;
    }
}



void ProphetRoutingLayer::setSyncingNeighbourInfoForNextRound()
{
    // loop thru syncing neighbor list and set for next round
    list<SyncedNeighbour*>::iterator iteratorSyncedNeighbour;
    iteratorSyncedNeighbour = syncedNeighbourList.begin();
    while (iteratorSyncedNeighbour != syncedNeighbourList.end()) {
        SyncedNeighbour *syncedNeighbour = *iteratorSyncedNeighbour;

        if (!syncedNeighbour->nodeConsidered) {

            // if neighbour not considered this time, then it means the
            // neighbour was not in my neighbourhood - so init all flags and timers

            syncedNeighbour->randomBackoffStarted = FALSE;
            syncedNeighbour->randomBackoffEndTime = 0.0;
            syncedNeighbour->neighbourSyncing = FALSE;
            syncedNeighbour->neighbourSyncEndTime = 0.0;
        }

        // setup for next time
        syncedNeighbour->nodeConsidered = FALSE;


        iteratorSyncedNeighbour++;
    }
}


void ProphetRoutingLayer::disconnectEncounterInterval(string nodeMACAddress)
{
    SyncedNeighbour *syncedNeighbour = getSyncingNeighbourInfo(nodeMACAddress);
    if (syncedNeighbour->nodeMACAddress == nodeMACAddress) {
	syncedNeighbour->lastSyncTime  = simTime().dbl();

    }

}



void ProphetRoutingLayer::setSyncingNeighbourInfoForNoNeighboursOrEmptyCache()
{
    // loop thru syncing neighbor list and set for next round
    list<SyncedNeighbour*>::iterator iteratorSyncedNeighbour;
    iteratorSyncedNeighbour = syncedNeighbourList.begin();
    while (iteratorSyncedNeighbour != syncedNeighbourList.end()) {
        SyncedNeighbour *syncedNeighbour = *iteratorSyncedNeighbour;
        
        syncedNeighbour->randomBackoffStarted = FALSE;
        syncedNeighbour->randomBackoffEndTime = 0.0;
        syncedNeighbour->neighbourSyncing = FALSE;
        syncedNeighbour->neighbourSyncEndTime = 0.0;
        syncedNeighbour->nodeConsidered = FALSE;

        iteratorSyncedNeighbour++;
    }
}


void ProphetRoutingLayer::updateDPT(string nodeMACAddress)
{
    //Step 1: Update the DP values  due to aging
    agingDP();
    list<DeliveryPredictability*>::iterator iteratorDeliveryPredictability;
    iteratorDeliveryPredictability = dpList.begin();
    list<DeliveryPredictability*>::iterator iteratorDeliveryPredictabilityReceived;
    iteratorDeliveryPredictabilityReceived =dpListReceived.begin();
    vector<string> destinationNodes;
    double encounteredDP = 0.0;
    // To update the DP of the encountered node
    while (iteratorDeliveryPredictability != dpList.end()) {
    	DeliveryPredictability *deliveryPredictability = *iteratorDeliveryPredictability ;
    	if(deliveryPredictability->nodeMACAddress == nodeMACAddress){

  	      encounteredDP = deliveryPredictability ->nodeDP;
               destinationNodes.push_back(nodeMACAddress);
	      // Condition :first Encounter
	      if (pEncounterFirst - encounteredDP > pFirstThreshold)
              { // Step 2: Update the DP of the encountered Node

	           encounteredDP = encounteredDP + ((1- encounteredDP) * pEncounterFirst );
	           deliveryPredictability->nodeDP = encounteredDP;
	           
	           break;
	              // Condition : Second or further Encounters
	       }else {// Check whether the Nodes is frequently visiting
                    
                   double intvl;
                   double Ityp;
              	   SyncedNeighbour *syncedNeighbour = getSyncingNeighbourInfo(nodeMACAddress);
              	   intvl= syncedNeighbour-> aEIValue ;
              	   Ityp = simTime().dbl() - syncedNeighbour-> lastSyncTime ;
		   //Ityp= syncedNeighbour-> aEIValue ;
              	   //intvl = simTime().dbl() - syncedNeighbour-> lastSyncTime ;
                   //cout << "check" << intvl <<"::::"<< Ityp << "\n";
              	   double pFinal = 0.0;
                   //cout<<intvl - Ityp <<"\n";
		   if (intvl - Ityp > pFirstThreshold )
 		   { //Non - Frequent
        		pFinal = pEncounterMax;
		    } else {
		     //Frequent visits so reduce the increment value
                        

			pFinal = pEncounterMax *(intvl / Ityp);
		     }
		     // Step 2: Update the DP of the encountered Node
                   //encounteredDP = encounteredDP + ((1- delta - encounteredDP) * pFinal );
		   encounteredDP = encounteredDP + ((1- encounteredDP) * pFinal );
		   deliveryPredictability->nodeDP = encounteredDP;
		  
                   break;
		}
    	}
	iteratorDeliveryPredictability++;
    }

    //Step 3: To update DPT For other Nodes in the neighborlist or DPT table

    iteratorDeliveryPredictability = dpList.begin();
    while (iteratorDeliveryPredictability != dpList.end()) {
        DeliveryPredictability *deliveryPredictability = *iteratorDeliveryPredictability ;
	while (iteratorDeliveryPredictabilityReceived != dpListReceived.end()) {
	    DeliveryPredictability *deliveryPredictabilityReceived = *iteratorDeliveryPredictabilityReceived ;

	    if(deliveryPredictability->nodeMACAddress == ownMACAddress)
	    {
		// No need to update its own value
          	break ;

	     }else if(deliveryPredictability->nodeMACAddress == nodeMACAddress){

                break ;
		// Encountered Node dp's already updated
	     }else if(deliveryPredictability->nodeMACAddress == deliveryPredictabilityReceived->nodeMACAddress){
	         // Step 3: Update the DP of the remaining node based on the Transitive Property
	         double encounteredNeighborDP = deliveryPredictabilityReceived->nodeDP;
	         double temp;
      	         temp = encounteredNeighborDP * encounteredDP * beta ;
		 double ownNeighborDP = deliveryPredictability ->nodeDP;
                 double diff = temp - ownNeighborDP ;
                 if (diff > pFirstThreshold)
	         {
	             // Update The next Hop address
      		     // Update the DPT value
	            
		     destinationNodes.push_back(deliveryPredictability->nodeMACAddress);
	             deliveryPredictability ->nodeDP =temp;
                  }
    		 break ;
	     }
	     iteratorDeliveryPredictabilityReceived++;
	}
        iteratorDeliveryPredictability++;
    }

  //Empty the received dplist
  list<DeliveryPredictability*> dpListReceived;
  sendDataMsg(destinationNodes, nodeMACAddress);
  
}

void ProphetRoutingLayer::finish()
{

    recordScalar("numEventsHandled", numEventsHandled);


	  // clear registered app list
    while (registeredAppList.size() > 0) {
        list<AppInfo*>::iterator iteratorRegisteredApp = registeredAppList.begin();
        AppInfo *appInfo= *iteratorRegisteredApp;
        registeredAppList.remove(appInfo);
        delete appInfo;
    }

    // clear stored data in the cache
    while (cacheList.size() > 0) {
        list<CacheEntry*>::iterator iteratorCache = cacheList.begin();
        CacheEntry *cacheEntry= *iteratorCache;
        cacheList.remove(cacheEntry);
        delete cacheEntry;
    }

    // clear synced neighbour info list
    list<SyncedNeighbour*> syncedNeighbourList;
    while (syncedNeighbourList.size() > 0) {
        list<SyncedNeighbour*>::iterator iteratorSyncedNeighbour = syncedNeighbourList.begin();
        SyncedNeighbour *syncedNeighbour = *iteratorSyncedNeighbour;
        syncedNeighbourList.remove(syncedNeighbour);
        delete syncedNeighbour;
    }


	  // clear DeliveryPredictability[own] table list
    list<DeliveryPredictability*> dpList;
	  while (dpList.size() > 0) {
        list<DeliveryPredictability*>::iterator iteratorDeliveryPredictability = dpList.begin();
        DeliveryPredictability *deliveryPredictability = *iteratorDeliveryPredictability;
        dpList.remove(deliveryPredictability);
        delete deliveryPredictability;
    }

	 // clear Received DeliveryPredictabilitytable list
   list<DeliveryPredictability*> dpListReceived;
	 while (dpListReceived.size() > 0) {
       list<DeliveryPredictability*>::iterator iteratorDeliveryPredictability = dpListReceived.begin();
       DeliveryPredictability *deliveryPredictability = *iteratorDeliveryPredictability;
       dpListReceived.remove(deliveryPredictability);
       delete deliveryPredictability;
   }
}

Conversation opened. 1 read message.

Skip to content
Using Gmail with screen readers
in:sent 

11 of 846
(no subject)
Inbox
x

KRISHANTH C <krishanth6666@gmail.com>
Tue, Oct 2, 11:21 AM (9 days ago)
to me

 delivery_Delay_Stats.py
 General-0-20181002-10:02:21-19288-log2.txt
2 Attachments

import os
import warnings
import sys
from collections import defaultdict


def file_open(filepath, mode):
    if not os.path.isfile(filepath):
        warnings.warn("No Such file exist in the" + filepath)
        file = open(filepath, 'x')
        file.close()
    file = open(filepath, mode, encoding="utf-8")
    return file


def main():
    messages = defaultdict(dict)
    message_generated = 0
    message_reached = 0
    accumulated_delay = 0.0
    tot_message_generate_count = 0
    tot_message_receive_count = 0
    tot_message_destination_count = 0
    message_generate_count = dict()
    message_receive_count = dict()
    message_destination_count = dict()
    message_transfer_count = dict()
    total_data_msg_transfer_count = 0
    if len(sys.argv) > 1:
        log_file = str(sys.argv[1])
        try:
            datafile = open(log_file, 'r', encoding="utf-8")
        except Exception as e:
            print("The i/p log file is missing")
            sys.exit(1)
    else:
        print("Enter the path of the log file to be parsed  \n Usage: delivery_delay_stats.py 'log file path ' ")
        sys.exit(1)
    for line in datafile:
        if "UI" in line and ("DM" in line):
            message_generated += 1
            messageInfo = defaultdict(list)
            words = line.split(">!<")
            messageInfo['creationTime'] = words[1]
            messageInfo['finalAddress'] = words[8]
            messageInfo['dataName'] = words[9]
            messageInfo['hopCount'] = words[12].strip()
            messageInfo['destinationReached'] = False
            messageInfo['duplicatetransmission'] = 0
            messageInfo['unnecessarytransmission'] = 0
            messageInfo['deliveryDelay'] = 0.0
            messages[words[9]] = messageInfo
            if words[3] in message_generate_count:
                message_generate_count[words[3]] += 1
            else:
                message_generate_count[words[3]] = 1

            if words[8] in message_destination_count:
                message_destination_count[words[8]] += 1
            else:
                message_destination_count[words[8]] = 1
		# unique message transfered  
        elif "LO" in line and ("DM" in line):
            total_data_msg_transfer_count += 1
            words = line.split(">!<")
            if words[9] in message_transfer_count:
                message_transfer_count[words[9]] += 1
            else:
                message_transfer_count[words[9]] = 1
            if words[7] == words[6]: 
                
                if words[9] not in receive_check:
                    receive_check.append(words[9])
                    if words[7] in message_receive_count:
                        message_receive_count[words[7]] += 1
                    else:
                        message_receive_count[words[7]] = 1
       

       
        elif "UO" in line and ("DM" in line):
            words = line.split(">!<")
            messageCheck = defaultdict(list)
            messageCheck = messages[words[8]]
            if words[3] == words[7]:
                if not messageCheck['destinationReached'] :
                    message_reached += 1
                    messageCheck['hopCount'] = words[11].strip()
                    messageCheck['destinationReached'] = True
                    check = float(words[1]) - float(messageCheck['creationTime'])
                    accumulated_delay += check
                    messageCheck['deliveryDelay'] = check
                    messages[words[8]] = messageCheck
                else:
                    messageCheck['duplicatetransmission'] += 1
                    messages[words[8]] = messageCheck
 
       
    # open file
    file = file_open('deliverydelay.txt', 'w')
    fi = file_open('Message_statistics_per_node.txt', 'w')
    fo = file_open('Transmission_statistics_per_message.txt', 'w')

    # Header
    fi.write('     Node Name     ' + "             No of messages generated    " + "No of messages to be reached    " + '            No of messages actually reached      \n')
    file.write('     Message Name     ' + "             Hopcount    " + "Destiantion Reached" + '              Delivery delay       \n')
    fo.write('     Message Name     ' + "             Duplicate_Transmission    "   + '      Reached     \n')

    # content
    unique_msg_transfered = 0
    total_duplicate_msg_reached = 0
    for keys, values in messages.items():
        if  values['destinationReached']:
            unique_msg_transfered += 1
            total_duplicate_msg_reached += values['duplicatetransmission']
           
        file.write(values['dataName'] + "             " + values['hopCount'] + "             " + str(values['destinationReached'])+"                      " + str(values['deliveryDelay']) + "\n")
        fo.write(values['dataName'] + "                  " + str(values['duplicatetransmission']) + "                      "  + "                        " + values['hopCount'] + "\n")

    for (ke, va), (key, value), (k, v) in zip(message_generate_count.items(), message_destination_count.items(), message_receive_count.items()):
        gen = 0
        rec = 0
        dest = 0 
        if ke in message_generate_count:
            tot_message_generate_count += message_generate_count[ke]
            gen = message_generate_count[ke]
        if ke in  message_receive_count:
            tot_message_receive_count += message_receive_count[ke]
            rec = message_receive_count[ke]
        if ke in message_destination_count:
            tot_message_destination_count += message_destination_count[ke]
            dest = message_destination_count[ke]

        fi.write(str(ke) + "                       " + str(gen) + "                              " + str(dest)+"                                             " + str(rec) + "\n")
    fi.write("------                               --------                          ------                                          -------  \n")
    fi.write("Total:                                " + str(tot_message_generate_count) + "                            " + str(tot_message_destination_count)+"                                           " + str(message_reached) + "\n")
    file.write("------------------------------------------------------")
    file.write("\nNo. of Messages generated : " + str(message_generated))
    file.write("\nNo. of Unique Messages transfered : " + str(len(message_transfer_count))) 
    file.write("\nNo. of Messages Reached : " + str(message_reached))
    file.write("\nNo. of Messages yet to Receive : " + str(message_generated - message_reached))
    file.write("\nAccumulated Delay " + str(accumulated_delay))
    file.write("\nAverage Delay " + str(accumulated_delay/message_reached))
    file.write("\nTotal duplicate messages reached : " + str(total_duplicate_msg_reached))
    file.write("\nTotal number of data message transfer : " + str(total_data_msg_transfer_count))
    file.write("\nDelivery Rate " + str((message_reached * 100 )/message_generated))

if __name__ == "__main__":
    main()
delivery_Delay_Stats.py
Displaying delivery_Delay_Stats.py.

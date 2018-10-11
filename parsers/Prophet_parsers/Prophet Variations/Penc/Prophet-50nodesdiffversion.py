# -*- coding: utf-8 -*-
"""
Created on Tue Sep 18 11:52:35 2018

@author: krishanth
"""


#!/usr/bin/python
# Script to plot the stats extracted related to sent
# packets.
#
# Author: Asanga Udugama (adu@comnets.uni-bremen.de)
# Date: 22-aug-2017

import numpy as np
import matplotlib.pyplot as plt

n_groups = 3
   
message_generated = (4200,4200,4200)
unique_msg_transfered = (2100, 2101, 2111)
unique_msg_reached = (2100, 2101, 2111)
duplicate_msg_reached = (534, 609, 621)
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
#Delivery_Rate = (63.10, 61.55, 58.5, 54.26, 50, 47.12  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, message_generated, bar_width,
                 alpha=opacity,
                 color='yellow', label='Message Generated')

rects2 = plt.bar(index + bar_width, unique_msg_transfered, bar_width,
                 alpha=opacity,
                 color='violet', label='Unique Msg Txed')

rects3 = plt.bar(index + (bar_width * 2), unique_msg_reached, bar_width,
                 alpha=opacity,
                 color='turquoise', label='Unique Msg Rxed')

rects4 = plt.bar(index + (bar_width * 3), duplicate_msg_reached, bar_width,
                 alpha=opacity,
                 color='red', label='Duplicate Msg')


plt.xlabel('pencountermax[DP]')
plt.ylabel('Message count')
plt.title('PRoPHET - Different Penc for 50 Nodes')
plt.xticks(index + (bar_width * 1.5), ('Penc = 0.7','Penc = 0.6', 'Penc = 0.5'))
plt.ylim(0, 4200)
plt.legend()

plt.tight_layout()
plt.show()

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

n_groups = 7
   

avg_duplicate_msg_reached = (70, 267, 1056, 2343, 6208, 11405, 22421)
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
#Delivery_Rate = (63.10, 61.55, 58.5, 54.26, 50, 47.12  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6


rects4 = plt.bar(index + (bar_width * 3), avg_duplicate_msg_reached, bar_width,
                 alpha=opacity,
                 color='red', label='Duplicate Msg')


plt.xlabel('simulation days ')
plt.ylabel('avg duplicate msg received for every 2100 msg generated')
plt.title('PRoPHET - Different sim days for 50 Nodes')
plt.xticks(index + 0.5 + (bar_width * 0.25), ('3.5','7',' 14',' 21','35','49', '70'))

plt.ylim(0, 23000)
plt.legend()

plt.tight_layout()
plt.show()

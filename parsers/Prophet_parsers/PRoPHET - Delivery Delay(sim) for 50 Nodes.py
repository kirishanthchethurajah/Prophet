# -*- coding: utf-8 -*-
"""
Created on Tue Sep 18 11:52:35 2018

@author: krishanth
"""

# -*- coding: utf-8 -*-
"""
Created on Tue Sep 18 10:47:05 2018

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
   
average_delivery_delay = (74963, 133841, 213046, 244071, 253981, 255295, 248081)
#Delivery_Rate = (63.10, 61.55, 58.5, 54.26, 50, 47.12  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, average_delivery_delay, bar_width,
                 alpha=opacity,
                 color='yellow', label='average_delivery_delay')

plt.xlabel('Simulation days')
plt.ylabel('Delivery Delay')
plt.title('PRoPHET - sim days for 50 Nodes')
plt.xticks(index + (bar_width * 0.125), (' 3.5','7',' 14',' 21','35','49', '70'))
plt.ylim(70000, 260000)

plt.legend()

plt.tight_layout()
plt.show()

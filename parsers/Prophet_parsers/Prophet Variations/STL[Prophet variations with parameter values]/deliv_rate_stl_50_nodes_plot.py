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

n_groups = 6
   
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
Delivery_Rate = (63.10, 61.55, 58.5, 54.26, 50, 47.12  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, Delivery_Rate, bar_width,
                 alpha=opacity,
                 color='violet', label='Delivery_Rate')

plt.xlabel('Standard Time Interval')
plt.ylabel('Delivery Rate')
plt.title('PRoPHET - Delivery Rate(STL) for 50 Nodes')
plt.xticks(index + (bar_width * 0.125), ('STL=225', 'STL=450', 'STL=900','STL=1800','STL=3600','STL-9000'))
plt.ylim(45, 66)

plt.legend()

plt.tight_layout()
plt.show()

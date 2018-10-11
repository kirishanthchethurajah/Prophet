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
   
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
Delivery_Rate = (31.14, 50, 73.94, 84.11 , 91.58, 94.43, 96.44) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, Delivery_Rate, bar_width,
                 alpha=opacity,
                 color='violet', label='Delivery_Rate')

plt.xlabel('simulation days')
plt.ylabel('Delivery Rate')
plt.title('PRoPHET - diff simulation days for 50 Nodes')
plt.xticks(index + 1 + (bar_width * 0.125), (' 3.5','7',' 14',' 21','35','49', '70'))
plt.ylim(30, 98)

plt.legend()

plt.tight_layout()
plt.show()

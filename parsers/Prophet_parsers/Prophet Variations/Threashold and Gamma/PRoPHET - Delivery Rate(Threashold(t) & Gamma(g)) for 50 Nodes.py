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
Delivery_Rate = (50, 65.38, 60.66 , 56.14, 62.24, 55.31, 52.26  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, Delivery_Rate, bar_width,
                 alpha=opacity,
                 color='violet', label='Delivery_Rate')

plt.xlabel('Threashold(t) & Gamma(g)')
plt.ylabel('Delivery Rate')
plt.title('PRoPHET - Delivery Rate(Threashold(t) & Gamma(g)) for 50 Nodes')
plt.xticks(index + (bar_width * 0.125), (' 0.1,  0.998',' 0.05,  0.9',' 0.05,  0.8',' 0.05,  0.7',' 0.1,  0.9',' 0.1,  0.8', ' 0.1,  0.7'))
plt.ylim(49, 66)

plt.legend()

plt.tight_layout()
plt.show()

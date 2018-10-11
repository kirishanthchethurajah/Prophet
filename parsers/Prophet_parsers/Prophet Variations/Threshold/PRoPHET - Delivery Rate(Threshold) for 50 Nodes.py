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

n_groups = 4
   
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
Delivery_Rate = (50, 53.74, 56.71, 57.12  ) 


fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, Delivery_Rate, bar_width,
                 alpha=opacity,
                 color='violet', label='Delivery_Rate')

plt.xlabel('Threashold')
plt.ylabel('Delivery Rate')
plt.title('PRoPHET - Delivery Rate(Threashold) for 50 Nodes')
plt.xticks(index + (bar_width * 0.125), ('0.1','0.05', '0.01', '0.0'))
plt.ylim(47, 60)

plt.legend()

plt.tight_layout()
plt.show()

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

n_groups = 3
   
#average_delivery_delay = (102703, 109863, 120299, 128261, 133841, 136118  )
prophet_delivery_rate = (50, 54.94,  53.52 ) 
epidemic_delivery_rate = (98.76, 99.25, 99.81)

fig, ax = plt.subplots()

index = np.arange(n_groups)
bar_width = 0.2

opacity = 0.6

rects1 = plt.bar(index, prophet_delivery_rate, bar_width,
                 alpha=opacity,
                 color='violet', label='prophet_delivery_rate')

rects1 = plt.bar(index+bar_width, epidemic_delivery_rate, bar_width,
                 alpha=opacity,
                 color='blue', label='epidemic_delivery_rate')

plt.xlabel('No. of Nodes')
plt.ylabel('Delivery Rate')
plt.title('PRoPHET VS EPIDEMIC')
plt.xticks(index + (bar_width * 0.125), ('50', '100', '150'))
plt.ylim(0, 100)

plt.legend()

plt.tight_layout()
plt.show()

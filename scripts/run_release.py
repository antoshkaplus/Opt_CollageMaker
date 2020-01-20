"""
  Seed = 1, N = 51
  Seed = 2, N = 74
  Seed = 3, N = 87
  Seed = 4, N = 74
  Seed = 5, N = 85
  Seed = 6, N = 92
  Seed = 7, N = 92
  Seed = 8, N = 87
  Seed = 9, N = 81
  Seed = 10, N = 69

"""

import os
import sys

os.system(r"java -jar CollageMakerVis.jar -exec 'python invoker.py %s'"
          r" -target ../dataset/300px -source ../dataset/100px" % " ".join(sys.argv[1:]))


#-Djava.awt.headless=true


#for i in range(0, 100):
#    os.system(r"java -Djava.awt.headless=true -jar CollageMakerVis.jar -exec 'python invoker.py'"
#          r" -target ./dataset/300px -source ./dataset/100px -novis -seed " + str(i) + " >> answer_12.txt ")

    
#for i in [200]:
#    os.system(r"java -jar CollageMakerVis.jar -exec ./../DerivedData/CollageMaker/Build/Products/Release/CollageMaker"
#          r" -target ./dataset/300px -source ./dataset/100px -seed " + str(i))



#i need another script that will read input stream,
#execute program and write to output stream
## Current very much a WIP
Right now you should be able to boot it, and it will just infinetly read the humidity data from register 0x00
 


## Future plans
- Make this very well commented so its really easy to follow for ppl in the workshop
- In src, the main.cpp file is where the code I currently am writing is. I plan to split it into headerfiles, and eventually remove the AM2320 library in ./pio/libdeps once i get a working one.
- At some point, after finishing the code and once I understand enough I wanna make a walkthrough video of how to make this library from scratch, as I think that would really help me to solidify my understanding of I2C and be a good experience to have. 

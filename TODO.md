# TODOs
- General
  - [ ] Documentation
  - [ ] Extend deploy.sh to allow develop releases for testing

 - Connection
  - [ ] MQTT 
  - [ ] UDP Server //postponed 
  - [ ] TCP Server //postponed 

- LED 
  - [ ] white/color temp -> rgbwwlib
  - [ ] 

# DONE

- General
  - [x] Load/Save settings
  - [x] catch dns request when not in AP mode and not able to connect
    - [x] implement dns server for sming
  - [x] reset settings when reset button low during startup
  - [x] refactor & code cleanup
  
- WEB Interface
  - [x] Initial Interface
  - [x] backend methods for ajax calls
  	- [x] scan networks
  	- [x] get settings
  	- [x] store settings
  	
- LED 
  - [x] interface for library
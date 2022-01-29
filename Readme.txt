Readme for PANCBC

Personal Area Network for Catheter Bag Control

Project consists of 2 embedded devices:
	- Controller
	- Receiver/Actuator
	
Control to have 2FA to avoid false positives
	- Capacitive touch detection >5s
	- Keyword detection of obscure/nonsensical phrase
	- BLE connection to Receiver/Actuator (security options to be explored)
	
Receiver/Actuator to consist of:
	- BLE connection to Controller
	- Low power MCU controlling
		- Latching solenoid pinch valve (silicone tubing below catheter bag pinched)
		
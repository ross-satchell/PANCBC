-------------------------------------
PANCBC Project Goal
-------------------------------------

This project will be considered a success if it meets all of the following criteria:

 - Sender & Receiver devices use low power techniques to maximize battery life
 - Sender & Receiver devices each alert when battery below 10%
 - Sender device successfully implements 2FA without false positives
 - BLE security protocol implemented between Sender and Receiver
 - Upon receiving appropriate command from Sender device:
	 - Receiver device successfully pinches silicone catheter tube to completely stop flow
	 - Receiver device successfully opens to allow full flow from catheter tubing
	 - Failsafe state in the pinched position
	 
---------------------------------------
Initial Solution Idea:
---------------------------------------
	
Receiver:
Catheter bag is attached to patients lower leg using velcro or similar. Proposed Receiver device sits directly downstream
of catheter bag, silicon tubing (part of catheter bag drain) fits in a slotted section of the
Receiver unit. An off-the-shelf latching solenoid pinch valve controls the silicon tubing in the slotted section.
Unit has secure BLE connection with only its paired Sender unit.
Audible low battery alert using piezo.
Multiple power rails may be needed: 3.3, 12V using Buck/Boost converters. 
Housing to be 3D printed and include a section to route silicone tubing through. 
			 
Stretch Goal (?): Extra piece of tubing to extend past footrest (about 1") to allow aiming, and is retracted after use so
	to be invisible when not in use.
Patient has muscle spasms resulting in knees being pulled to chest. There must be sufficient slack between flow controller
	and tube extender to avoid tube being pulled out.

Sender:
Small handheld unit uses 2 MCUs. An 8-bit 2FA (Capacitive touch pad & keyword spotting) to trigger a send order. 
A low power 8-bit MCU controls capacitive touch and power gates a 32-bit MCU running keyword spotting model.
Keyword spotting model to be developed using Edge Impulse or similar service. Audible low battery alert using piezo.
Capacitive touch pad likely to be a small exposed PCB, with size usable by patient approx 7cm x 5xm.
Sender unit size to be approx similar to deck of cards, so patient can handle it easily.

Latching Solenoid Pinch Valve:
https://www.images.pumps-valves.eu/fileadmin/datenblaetter/bmt-en/Pinch-valves-to-2mm.pdf

--------------------------------------
Project Behaviour:
---------------------------------------
Visio flowchart to be created and uploaded.

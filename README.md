# IOC
IoT assignments and a project
Each labs is baout a particular aspect of IoT, they contain a report explaining what we did and answering some mandatory questions, the reports are in french but I'll list the projects in english here
- Lab 1 : an introduction to our tools and basic microcontrollers mechanisms like **GPIO interruptions** and making LEDs blink at different frequency, virtual and physical memory, retrieving data from GPIO
- Lab 2 : an introduction to kernel drivers, add parameters, load/unload the module and access a GPIO through the module
- Lab 3 : manipulation of an LCD screen and write a driver to manipulate the cursor (write on two lines fo example)
- Lab 4 : introduciton to Arduino and ESP32, the aim is to write a multi task application, we had to initialize our devices and send different messages, use the OLED screen on the ESP32 and implement a **mailbox** for our tasks to communicate.
- Lab 5 : implement a very simple Web server enabing the user to access our ESP32 captors, this is a way to introduce us slowly to this notion in order to prepare us for the upcoming project. We saw how to make a C program and python program communicate together using a FIFO so we can retrieve data from the captors and send them to web server. From the server we can also turn on or off the LED lights.
- Lab 6 : introduction to TCP/IP posix sockets for the server and implementing a voting application. For this one it was necessary to implement a hashmap because we didn't have one in the standard library
- project : a mix of all those notions : a web server from which we can access captors values and send commands to actuators on the ESP32
  - turn on/off LEDs
  - play a song through the buzzer
  - Access the light sensor's values   

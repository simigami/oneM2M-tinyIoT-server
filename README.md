# tinyIoT - very light oneM2M server

oneM2M is a global partnership project founded in 2012 to create a global technical standard for interoperability concerning the architecture, API specifications, security and enrolments solutions for Internet of Things (IoT) technologies based on requirements contributed by its members. The standardised specifications produced by oneM2M enable an Eco-System to support a wide range of applications and services such as smart cities, smart grid, connected car, home automation, public safety, and health.

tinyIoT is a secure, fast, lightweight and very flexible oneM2M service layer platform compliant with oneM2M specifications. tinyIoT uses memory and CPU efficiently and has low resource use. tinyIoT is implemented in C and uses lightweight open source components for database (Berkeley DB), JSON parser (cJSON), and http server (foxweb). tinyIoT also comes with a web-based dashboard implemented using Vue. 

tinyIoT supports the following features: 

- Registration of IoT 
- Application Entity (AE), Container (CNT), contentInstance (CIN) resources
- CRUD operations
- Subscription and Notification
- Discovery
- Bindings: HTTP and MQTT (CoAP will be added in 2023 first quarter)
- Group management (to be added in 2023 first quarter)

## Operating System

Linux Ubuntu 22.04

## Quick start

1. Configure your running environment with the `config.h` file.
2. Run `./server`
3. If there is no executable file, Run `make`

## make dependency

1. Berkeley DB 18.1.32 → https://mindlestick.github.io/TinyIoT/build/html/installation.html (guide)
2. cJSON

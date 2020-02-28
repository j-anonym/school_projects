#IPK project 1

### Implementation
Project implements simple `client`, that obtain and parse forecast data in `Java`. 
`HTTP protocol` is used to get data from server. Program creates `socket` and sends request to server via `output stream`.
To read data, program uses `input stream`. Then `HTTP header` is checked if answer is correct.
If not program returns exit code 1. Otherwise program creates new JSON file using included `JSON library` and then
data is parsed and displayed to output.

### Usage
User needs to sign up to get APIkey from OpenWeather  
**Steps**  

- Register your free account at https://home.openweathermap.org/users/sign_up
- Sign in
- Go to https://home.openweathermap.org/api_keys and generate new APIkey
- Wait for your APIkey to be confirmed
- Use your APIkey that can be found in your profile


####How to start program:
        
    In directory with Makefile type 'make run api-key=<YOURAPIKEY> city=<YOURCITY>' into terminal

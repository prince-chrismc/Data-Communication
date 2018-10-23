# File-Server
This is a lightweight file server which allows for exploring and modifying a file structure through a browser.

>*To view this [document](https://github.com/prince-chrismc/Data-Communication/blob/master/Assignments/File-Server/README.md) properly, click on the link provided.*

### Table Of Contents
* [Contributors](#contributors)
* [Test Suite](#Test-Suite)

## Contributors
**Student** | **ID**
:---:| ---
Christopher McArthur | `40004257`

## Test Suite
These tests are made to be run with the accommanying Curl application on a Linux system.

#### Get
```sh
./Curl.run get http://127.0.0.1:8080/            # 200
./Curl.run get http://127.0.0.1:8080/Hello.txt   # 200 # Inline text/plain # Default
./Curl.run get http://127.0.0.1:8080/post.json   # 200 # Inline application/json for browser
./Curl.run get http://127.0.0.1:8080/index.html  # 200 # Inline application/html for browser
./Curl.run get http://127.0.0.1:8080/example.png # 200 # Inline content for browser
./Curl.run get http://127.0.0.1:8080/FolderA     # 200
./Curl.run get http://127.0.0.1:8080/FolderA/FolderB      # 200
./Curl.run get http://127.0.0.1:8080/FolderA/.hidden-dir  # 200
./Curl.run get http://127.0.0.1:8080/FolderA/.hidden-file # 200
./Curl.run get http://127.0.0.1:8080/Windows%20Users%Who%20Like%20Spaces # 200
./Curl.run get http://127.0.0.1:8080/UnknowPath # 404
./Curl.run get http://127.0.0.1:8080/../..      # 403
./Curl.run get http://127.0.0.1:8080/../Home    # 403 # it's a feature not a bug!
./Curl.run get http://127.0.0.1:8080/Home/../.. # 200 # Okay... This is a bug
```
#### Post
```sh
# Failure to create will return 409 with an error message if the OS failed.
./Curl.run post -d "New Text File\r\n" http://127.0.0.1:8080/Hello-World.txt # 201
./Curl.run post http://127.0.0.1:8080/Hello/ # 409 # Empty Director
./Curl.run post http://127.0.0.1:8080/Hello # 201
./Curl.run post http://127.0.0.1:8080/Hello/.hidden-dir # 201
./Curl.run post -d "New Dir/ Text File\r\n" http://127.0.0.1:8080/Hello/World.txt # 201
./Curl.run post -d "{}" http://127.0.0.1:8080/post.json  # 201 # Inline application/json for browsers
./Curl.run post -d "{\"error\":{}}" http://127.0.0.1:8080/post.json # 501 # Overwrite not implemented
# TO DO : More use cases
```

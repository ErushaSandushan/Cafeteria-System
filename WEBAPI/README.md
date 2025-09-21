# Web API (Not Smooth)

Please clone below repos
    - `https://github.com/chriskohlhoff/asio.git`
    - `https://github.com/CrowCpp/Crow.git`

How to complie : `g++ -std=c++17 main.cpp -o API -I Crow/include -lsqlite3 -I asio/include -lws2_32 -lmswsock`

Please note that your folder path needs to arranged as below
```bash
/WEBAPI/   
    -- Crow/
    -- asio/
    -- templates/
    -- main.cpp
```

![](../Imgs/API.png)
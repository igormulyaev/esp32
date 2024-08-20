idf_component_register (
    SRCS 
        "main.cpp"
        "StaticObjects.cpp"
        "WiFiStation/WiFiStation.cpp"
        "WiFiStation/NVS.cpp"
        "HttpServer/HttpServerBase.cpp"
        "HttpServer/HttpUriHandlerBase.cpp"
        "HttpServer/HttpUriTest.cpp"
        "HttpsClient/HttpsClient.cpp"
        "TelegramBot/TelegramBot.cpp"
        "TelegramBot/CjsonHandler.cpp"
        "TelegramBot/AnswerParserBase.cpp"
        "TelegramBot/AnswerParserGetMe.cpp"
        "secret_data.cpp"
    INCLUDE_DIRS "."
    EMBED_TXTFILES telegram.org.pem
    REQUIRES
        nvs_flash
        esp_event
        esp_http_server
        esp_https_server
        esp_http_client
        esp_wifi
        json
)

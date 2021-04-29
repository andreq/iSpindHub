#include "web.h"
AsyncWebServer server(80);
//extern Adafruit_ST7735 tft;
const char *filename = "/config.json";
Config config;

extern const size_t capacitySerial = 3 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(11);
extern const size_t capacityDeserial = capacitySerial + 810;

void initWebServer()
{
    setRegPageAliases();
    setActionPageHandlers();
    setJsonHandlers();
    setSettingsAliases();
    server.onNotFound([](AsyncWebServerRequest *request) {
        Log.verbose(F("Serving 404." CR));
        request->send(404, F("text/plain"), F("404: File not found."));
    });

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    server.begin();

    Log.notice(F("Async HTTP server started on port 80" CR));
}

void setRegPageAliases()
{
    // Regular page aliases

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.htm").setCacheControl("max-age=600");
    server.serveStatic("/index.htm", LittleFS, "/").setDefaultFile("index.htm").setCacheControl("max-age=600");
    server.serveStatic("/about/", LittleFS, "/").setDefaultFile("about.htm").setCacheControl("max-age=600");
    server.serveStatic("/help/", LittleFS, "/").setDefaultFile("help.htm").setCacheControl("max-age=600");
    server.serveStatic("/settings/", LittleFS, "/").setDefaultFile("settings.htm").setCacheControl("max-age=600");
    server.serveStatic("/wifi/", LittleFS, "/").setDefaultFile("wifi.htm").setCacheControl("max-age=600");
}
void setActionPageHandlers()
{   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Log.notice("Connected on /");
        request->send(200, F("text/plain"), "Hello World");
    });

}
void setJsonHandlers()
{
        server.on("/ispindel", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Used to handle the json coming from iSpindel
         }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            //for (size_t i = 0; i < len; i++) {
            //    Serial.write(data[i]);
            //}

            StaticJsonDocument<300> jdoc;
            //ReadLoggingStream loggingStream(data, Serial);
            //DeserializationError error = deserializeJson(jdoc, loggingStream);
            DeserializationError error = deserializeJson(jdoc, (const char*)data);
            Log.verbose(F("Parsing json from ispindel.\n"));
            if (!error) {
                //tft.fillRect(0,0,128,20,ST7735_BLACK);
                // Get Data from JSON 
                float data_size = jdoc.size();
                const char* name = jdoc["name"];
                //long id = jdoc["ID"];
                float angle = jdoc["angle"];
                const char* t_unit = jdoc["temp_units"];
                float temp = jdoc["temperature"];
                float battery = jdoc["battery"];
                // Output iSpindel Name
                //Serial.println("name of file");
                //Serial.println(name);
                //Serial.println(temp);
                
                // Build and open file
                String fname = String("/data/") + String(name) + String(".csv");
                //Serial.println(fname);
                File iSpindLog = LittleFS.open(fname, "a");    
                iSpindLog.print(getDTS());
                iSpindLog.print(",");
                iSpindLog.print(name);
                iSpindLog.print(",");
                iSpindLog.print(angle,4);
                iSpindLog.print(",");
                iSpindLog.print(temp,2);
                iSpindLog.print(",");
                iSpindLog.print(battery,4);
                
                // Check latest data Vs current TODO
                if (data_size > 5){
                    float gravity = jdoc["gravity"];
                    iSpindLog.print(",");
                    iSpindLog.print(gravity,4);
                    float interval = jdoc["interval"];
                    iSpindLog.print(",");
                    iSpindLog.print(interval,1);
                    float rssi = jdoc["RSSI"];
                    iSpindLog.print(",");
                    iSpindLog.print(rssi,1);
                    iSpindLog.print(",");
                    iSpindLog.write(t_unit);
                    iSpindLog.print(",");
                    iSpindLog.print("\r");
                    iSpindLog.close();
                }
                else{
                    iSpindLog.print(",");
                    iSpindLog.print(",");
                    iSpindLog.print(",");
                    iSpindLog.print(",");
                    iSpindLog.write(t_unit);
                    iSpindLog.print(",");
                    iSpindLog.print("\r");
                    iSpindLog.close();
                }
                
                request->send(200, "text/plain", "");
            } else {
            Log.verbose(F("deserializeJson() failed: "));
            Log.verbose(error.f_str());
            request->send(404, "text/plain", "");
            }
    });
        server.on("/iSpindInfo/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Used to provide the Bubbles json
        Log.verbose(F("Sending /iSpindInfo/." CR));

        FSInfo fs_info;
        LittleFS.info(fs_info);
        Dir dir = LittleFS.openDir("/data");
        String file_info = "{";
        while (dir.next()) {
            //Porcessing One file at at time
            String f_name = dir.fileName();
            if(dir.fileSize()) {
                File file = dir.openFile("r");
                time_t cr = file.getCreationTime();
                //Serial.println(cr);
                time_t lw = file.getLastWrite();
                //Serial.println(lw);
                //Get Last Readings
                String temp = file.readStringUntil('\r');
                int line_len = temp.length()+1;
                int file_size = file.size();
                float num_line = file_size/line_len;
                file.seek(line_len,SeekEnd);
                String lastData  = file.readString();
                //String lastData = get_last_value(file.readString());
                //Serial.println(iSpinData);
                file.close();
                //Store Name
                file_info+= "\"" + f_name+ "\"";
                file_info+= ": { \"created\":\"";
                struct tm * tmstruct = localtime(&cr);
                char t_format[25];
                sprintf(t_format,"%d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                //store creation date
                file_info+=String(t_format);
                tmstruct = localtime(&lw);
                sprintf(t_format,"%d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                file_info+= "\", \"last_updated\":\"";
                //store last updated
                file_info+= String(t_format);
                // store file size
                file_info+= "\", \"file_size\":\"";
                file_info+=String(file_size);
                // store number of lines
                file_info+= "\", \"lines_number\":\"";
                file_info+=String(int(num_line));
                //Store Last Readings
                int str_len = lastData.length() +1;
                int count = 0;
                int idx;
                int mov_idx = 0;
                String array_data[10] = {};
                for (idx = 0; idx <= str_len; idx++)
                {
                    if (lastData[idx] == ',')
                    {   //splitData[count] = lastData.substring(mov_idx,idx-1);
                    array_data[count] = lastData.substring(mov_idx,idx);
                    mov_idx = idx+1;
                    count++;
                    }
                }
                file_info+="\", \"SG\":\"";
                file_info+=array_data[5];
                file_info+="\", \"T\":\"";
                file_info+=array_data[3] + " °" + array_data[8];
                file_info+="\", \"B\":\"";
                file_info+=array_data[4];
                file_info+="\", \"RSSI\":\"";
                file_info+=array_data[7];
                file_info+= "\"},";
            }
        }
        int size_info = file_info.length()-1;
        file_info.remove(size_info,1);
        file_info+= "}";
        //Serial.print(file_info);
        //const size_t capacity = JSON_OBJECT_SIZE(8) + 210;
        //StaticJsonDocument<capacity> doc;
        //JsonObject root = doc.to<JsonObject>();
        //Serial.print(root);
        //String json;
        //serializeJsonPretty(doc, json);

        //request->send(200, F("application/json"), json);
        request->send(200, F("application/json"), file_info);
    });
};


void setSettingsAliases()
{
        server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
            Log.verbose(F("On arrive dans le Download" CR));
            if (request->hasParam("file")){
                request->send(LittleFS, "/data/" +  request->getParam("file")->value(), "text/plain", true);

            }
            request->send(404, F("text/plain"), F("File Not Found."));
    });
    
    server.on("/settings/urltarget/", HTTP_POST, [](AsyncWebServerRequest *request) {
        Log.verbose(F("Processing post to /settings/urltarget/." CR));
        if (handleURLTargetPost(request))
        {
            request->send(200, F("text/plain"), F("Ok"));
        }
        else
        {
            request->send(500, F("text/plain"), F("Unable to process data"));
        }
    });

    server.on("/settings/urltarget/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        Log.verbose(F("Invalid method to /settings/tapcontrol/." CR));
        request->send(405, F("text/plain"), F("Method not allowed."));
    });


    server.on("/settings/brewfathertarget/", HTTP_POST, [](AsyncWebServerRequest *request) {
        Log.verbose(F("Processing post to /settings/brewersfriendtarget/." CR));
        if (handleBrewfatherTargetPost(request))
        {
            request->send(200, F("text/plain"), F("Ok"));
        }
        else
        {
            request->send(500, F("text/plain"), F("Unable to process data"));
        }
    });

    server.on("/settings/brewfathertarget/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        Log.verbose(F("Invalid method to /settings/brewersfriendtarget/." CR));
        request->send(405, F("text/plain"), F("Method not allowed."));
    });

}
bool handleURLTargetPost(AsyncWebServerRequest *request) // Handle URL Target Post
{
    // Loop through all parameters
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost())
        {
            // Process any p->name().c_str() / p->value().c_str() pairs
            const char *name = p->name().c_str();
            const char *value = p->value().c_str();
            Log.verbose(F("Processing [%s]:(%s) pair." CR), name, value);

            // URL Target settings
            //
            if (strcmp(name, "urltargeturl") == 0) // Change Target URL
            {
                if (strlen(value) == 0)
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied.  Disabling Url Target." CR), name, value);
                    strlcpy(config.urltarget.url, value, sizeof(config.urltarget.url));
                }
                else if ((strlen(value) < 3) || (strlen(value) > 128))
                {
                    Log.warning(F("Settings update error, [%s]:(%s) not applied." CR), name, value);
                }
                else
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied." CR), name, value);
                    strlcpy(config.urltarget.url, value, sizeof(config.urltarget.url));
                }
            }
            if (strcmp(name, "urlfreq") == 0) // Change Target URL frequency
            {
                if ((atoi(value) < 1) || (atoi(value) > 60))
                {
                    Log.warning(F("Settings update error, [%s]:(%s) not applied." CR), name, value);
                }
                else
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied." CR), name, value);
                    config.urltarget.freq = atoi(value);
                    config.urltarget.update = true;
                }
            }
        }
    }
    if (saveConfig())
    {
        return true;
    }
    else
    {
        Log.error(F("Error: Unable to save tap configuration data." CR));
        return false;
    }
}

bool handleBrewfatherTargetPost(AsyncWebServerRequest *request) // Handle Brewfather Target Post
{
    // Loop through all parameters
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost())
        {
            // Process any p->name().c_str() / p->value().c_str() pairs
            const char *name = p->name().c_str();
            const char *value = p->value().c_str();
            Log.verbose(F("Processing [%s]:(%s) pair." CR), name, value);

            // Brewer's Friend target settings
            //
            if (strcmp(name, "brewfatherkey") == 0) // Change Brewfather key
            {
                if (strlen(value) == 0)
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied. Disabling Brewfather target." CR), name, value);
                    strlcpy(config.brewfather.key, value, sizeof(config.brewfather.key));
                }
                else if ((strlen(value) < 10) || (strlen(value) > 64))
                {
                    Log.warning(F("Settings update error, [%s]:(%s) not applied." CR), name, value);
                }
                else
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied." CR), name, value);
                    strlcpy(config.brewfather.key, value, sizeof(config.brewfather.key));
                }
            }
            if (strcmp(name, "brewfatherfreq") == 0) // Change Brewfather frequency
            {
                if ((atoi(value) < 15) || (atoi(value) > 120))
                {
                    Log.warning(F("Settings update error, [%s]:(%s) not applied." CR), name, value);
                }
                else
                {
                    Log.notice(F("Settings update, [%s]:(%s) applied." CR), name, value);
                    config.brewfather.freq = atoi(value);
                    config.brewfather.update = true;
                }
            }
        }
    }
    if (saveConfig())
    {
        return true;
    }
    else
    {
        Log.error(F("Error: Unable to save tap configuration data." CR));
        return false;
    }
}

bool saveConfig()
{
    return saveFile();
}

bool saveFile()
{
    // Saves the configuration to a file on File System
    File file = LittleFS.open(filename, "w");
    if (!file)
    {
        file.close();
        return false;
    }

    // Serialize JSON to file
    if (!serializeConfig(file))
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}


bool deserializeConfig(Stream &src)
{
    // Deserialize configuration
    DynamicJsonDocument doc(capacityDeserial);

    // Parse the JSON object in the file
    DeserializationError err = deserializeJson(doc, src);

    if (err)
    {
        // We really don;t care if there's an err, the file should be created anyway
        config.load(doc.as<JsonObject>());
        return true;
    }
    else
    {
        config.load(doc.as<JsonObject>());
        return true;
    }
}

bool serializeConfig(Print &dst)
{
    // Serialize configuration
    DynamicJsonDocument doc(capacitySerial);

    // Create an object at the root
    JsonObject root = doc.to<JsonObject>();

    // Fill the object
    config.save(root);

    // Serialize JSON to file
    return serializeJsonPretty(doc, dst) > 0;
}

void Config::save(JsonObject obj) const
{
    // Add Target object
    urltarget.save(obj.createNestedObject("urltarget"));
    // Add Brewfather object
    brewfather.save(obj.createNestedObject("brewfather"));
}

void Config::load(JsonObjectConst obj)
{
    // Load all config objects
    urltarget.load(obj["urltarget"]);
    brewfather.load(obj["brewfather"]);
}

void URLTarget::save(JsonObject obj) const
{
    obj["url"] = url;
    obj["freq"] = freq;
    obj["update"] = update;
}

void URLTarget::load(JsonObjectConst obj)
{
    // Load URL Target configuration
    //
    if (obj["url"].isNull())
    {
        strlcpy(url, "", sizeof(url));
    }
    else
    {
        const char *tu = obj["url"];
        strlcpy(url, tu, sizeof(url));
    }

    if (obj["freq"].isNull())
    {
        freq = 2;
    }
    else
    {
        int f = obj["freq"];
        freq = f;
    }

    if (obj["update"].isNull())
    {
        update = false;
    }
    else
    {
        bool u = obj["update"];
        update = u;
    }
}

void KeyTarget::save(JsonObject obj) const
{
    obj["channel"] = channel;
    obj["key"] = key;
    obj["freq"] = freq;
    obj["update"] = update;
}

void KeyTarget::load(JsonObjectConst obj)
{
    // Load Key-type configuration
    //
    if (obj["channel"].isNull())
    {
        channel = 0;
    }
    else
    {
        int c = obj["channel"];
        channel = c;
    }

    if (obj["key"].isNull())
    {
        strlcpy(key, "", sizeof(key));
    }
    else
    {
        const char *k = obj["key"];
        strlcpy(key, k, sizeof(key));
    }

    if (obj["freq"].isNull())
    {
        freq = 15;
    }
    else
    {
        int f = obj["freq"];
        freq = f;
    }

    if (obj["update"].isNull())
    {
        update = false;
    }
    else
    {
        bool u = obj["update"];
        update = u;
    }
}

// return HTTP ok
void returnOK();
// retunr HTTP fail
void returnFail(String);

// open sdcard
bool loadFromSdCard(String);

void handleFileUpload();
void handleDelete();
void handleCreate();
void handleNotFound();
void handleClient();
void printDirectory();
void deleteRecursive(String path);


// Initialize
void WifiServerinit();

void setSDStatus(bool);

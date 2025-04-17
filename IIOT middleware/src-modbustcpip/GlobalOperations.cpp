#include "GlobalOperations.h"

// Returns the last token after the specified separator in a given string.
std::string GetLastToken(std::string value, std::string seprator)
{
    std::size_t found = value.find_last_of(seprator);
    std::cout << " file: " << value.substr(found + 1) << '\n';
    return value.substr(found + 1);
}

// Reads a configuration file and parses it into a JSON object.
nlohmann::json ReadAndSetConfiguration(std::string fileName)
{
    std::ifstream file(fileName.c_str());
    nlohmann::json configurationJson;

    if (file)
    {
        // Read entire file content into a string
        std::string fileContentString((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
        std::cout << "configuration file content : " << fileContentString << std::endl;

        try
        {
            // Parse string into JSON object
            configurationJson = nlohmann::json::parse(fileContentString.c_str());
            std::cout << "configuration file content : " << configurationJson << std::endl;
        }
        catch (nlohmann::json::exception &e)
        {
            std::cout << e.id << " : " << e.what() << std::endl;
        }
    }
    else
    {
        std::cout << "File Not found : " << fileName << std::endl;
    }

    return configurationJson;
}

// Writes the provided JSON object into a specified configuration file.
bool WriteConfiguration(std::string fileName, nlohmann::json contentObj)
{
    std::ofstream outdata;

    outdata.open(fileName.c_str()); // Opens file for writing
    if (!outdata)
    {
        std::cout << "Error: file could not be opened" << std::endl;
        return false;
    }

    outdata << contentObj; // Write JSON content to file
    outdata.close();
    return true;
}

// Returns a current timestamp string in ISO 8601 format with millisecond precision.
std::string GetTimeStamp()
{
    char buf[100];
    char buf1[100];
    int millisec;
    struct tm *tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL); // Get current time
    millisec = lrint(tv.tv_usec / 1000.0); // Convert microseconds to milliseconds

    if (millisec >= 1000)
    {
        // Handle case where milliseconds round up to next second
        millisec -= 1000;
        tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec); // Convert seconds to local time structure
    strftime(buf, 26, "%Y-%m-%dT%H:%M:%S", tm_info); // Format time string
    sprintf(buf1, "%s.%03dZ", buf, millisec); // Append milliseconds and 'Z'
    return buf1;
}

// Returns the process ID (PID) of a given process by its name.
long GetProcessIdByName(std::string processName)
{
    char line[100];
    std::string pidCommand = "pidof ";

    // On certain systems, "-s" is used to return a single PID
    #ifdef KEMSYS_GATEWAY
        pidCommand += processName;
    #else
        pidCommand += "-s " + processName;
    #endif

    pid_t pid = 0;
    FILE *cmd = popen(pidCommand.c_str(), "r"); // Run the command

    if (cmd != NULL)
    {
        fgets(line, 100, cmd); // Read output from command
        pid = strtoul(line, NULL, 10); // Convert string to long
        pclose(cmd);
    }

    return pid;
}

// Rounds a float to 2 decimal places and returns it.
float round_value(float var)
{
    char str[40];
    sprintf(str, "%.2f", var); // Format float to 2 decimal places as string
    sscanf(str, "%f", &var);   // Convert string back to float
    std::cout << "value : " << var << "\n\n";
    return var;
}

// Generates and returns the current time in epoch format (seconds since Jan 1, 1970).
// Useful for timestamps where a compact format is needed.
std::string GenerateCurrentDateTimeInEPOCH()
{
    char buf1[100];
    long ms;
    time_t secs;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec); // Get current real time
    secs = spec.tv_sec;

    sprintf(buf1, "%s", secs); // Convert to string
    return buf1;
}

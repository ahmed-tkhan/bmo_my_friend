#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#include <Arduino.h>
#include <functional>
#include <vector>

class SerialCommand {
public:
    using Handler = std::function<void(const String& args)>;

    SerialCommand();
    void begin(Stream* stream);
    void registerCommand(const String& name, Handler cb);
    void process();
    void reply(const String& s);

private:
    struct Cmd { String name; Handler cb; };
    Stream* stream_;
    String lineBuf_;
    std::vector<Cmd> cmds_;
    static String toUpperCopy(const String& s);
};

#endif // SERIAL_COMMAND_H

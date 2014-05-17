//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#include "Metadata.h"
#include <fileref.h>
#include <tag.h>
#include <iostream>

#include <cstdio>
std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

// deal with quotes etc in shell
static std::string escape(const string& value) {
    std::string s(value);
    std::string out = "";
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if ((unsigned char)c < 31)
            continue;

        switch (c) {
            case '"' : out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case ' ': out += "\\ " ; break;
            default:
                out += c;
        }
    }

    return out;
}

Metadata::Metadata(const string& file) : _TagsFilled(false),
                                         _Filename(file), _Artist(""), 
                                         _Album(""), _Title(""), _Genre(""), 
                                         _Bitrate(0), _SampleRate(0), _Seconds(0) {
    if (file != "stdin") {
        // TODO: Consider removing the path from the filename -- not sure if we can do this in a platform-independent way.
        TagLib::FileRef f(_Filename.c_str());

        TagLib::Tag* tag = f.isNull() ? NULL : f.tag();
        if (tag != NULL) {
            _Artist = tag->artist().to8Bit(true);
            _Album = tag->album().to8Bit(true);
            _Title = tag->title().to8Bit(true);
            _Genre = tag->genre().to8Bit(true);
            _TagsFilled = true;
        }

        TagLib::AudioProperties* properties = f.isNull() ? NULL : f.audioProperties();
        if (properties != NULL) {
            _Bitrate = properties->bitrate();
            _SampleRate = properties->sampleRate();
            _Seconds = properties->length();
        }
        if (_Seconds == 0) {
            std::string command = "avprobe -show_format " + escape(_Filename) 
                + " | grep duration | sed 's/.*=//' 2> /dev/null";
            _Seconds = atof( exec(command.c_str()).c_str() );
        }
    }
}


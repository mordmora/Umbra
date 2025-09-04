#include "umbra/preprocessor/Sanitize.h"

#include<string>
namespace umbra {

    Encoding Sanitizer::detectEncoding(std::string& inputBytes){
	if(inputBytes.size() >= 3){
		if((unsigned char)inputBytes[0] == 0xEF && (unsigned char)inputBytes[1] == 0xBB && (unsigned char)inputBytes[2] == 0xBF){
			return umbra::Encoding::UTF8;
		}

		if((unsigned char)inputBytes[0] == 0xFE && (unsigned char)inputBytes[1] == 0xFF){
			return umbra::Encoding::UTF16BE;
		}

		if((unsigned char)inputBytes[0] == 0xFF && (unsigned char)inputBytes[1] == 0xFE){
			return umbra::Encoding::UTF16LE;
		}	
	}
	return umbra::Encoding::UNKNOWN;
    }	    
	
    std::string Sanitizer::convertInternalEncoding(std::string& inputBytes){

    }


   
}

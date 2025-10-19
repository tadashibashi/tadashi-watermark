#include "errors.h"
#include <lame/lame.h>

const char *get_lame_error_str(int lame_err) {
  	if (lame_err >= 0)
    	return "no error";

  	switch (lame_err) {
    	case LAME_GENERICERROR: 	return "generic error";
    	case LAME_NOMEM:        	return "out of memory";
    	case LAME_BADBITRATE:   	return "bad bit rate";
    	case LAME_BADSAMPFREQ:      return "bad samplerate";
    	case LAME_INTERNALERROR:    return "internal error";
    	case FRONTEND_READERROR:    return "read error";
    	case FRONTEND_WRITEERROR:   return "write error";
    	case FRONTEND_FILETOOLARGE: return "file too large";
  		default: 					return "unknown error";
  	}
}
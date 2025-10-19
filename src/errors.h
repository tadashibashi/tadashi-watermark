#pragma once

enum ErrCode {
    ERR_OK,
    ERR_FILE_READ,
    ERR_LAME_INIT,
    ERR_LAME_ENCODE,
    ERR_MA_DECODE,
    ERR_MA_RESAMPLER_INIT,
    ERR_MA_RESAMPLER_PROCESS,
};

const char *get_lame_error_str(int lame_err);
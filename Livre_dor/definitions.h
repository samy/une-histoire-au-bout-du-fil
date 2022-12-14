#define SCI_RECGAIN    0x0D /* (AICTRL1)  VS1063, VS1053, VS1033, VS1003 */
#define SCI_RECGAIN    0x0D /* (AICTRL1)  VS1063, VS1053, VS1033, VS1003 */
#define SCI_RECMODE    0x0F /* (AICTRL3)  VS1063, VS1053 */
#define RM_53_FORMAT_IMA_ADPCM  0x0000
#define RM_53_ADC_MODE_LEFT             0x0002
#define RM_53_ADC_MODE_JOINT_AGC_STEREO 0x0000
#define SCI_RECWORDS   0x09
#define REC_BUFFER_SIZE 512
#define SCI_RECDATA    0x08 /* (HDAT0)    VS1063 */
#define SCI_MODE        0x00
#define SM_LINE1          (1<<14) /* VS1063, VS1053 */
#define SM_ADPCM          (1<<12) /* VS1053, VS1033, VS1003 */
#define SM_RESET          (1<< 2)
#define SCI_CLOCKF      0x03
#define SEEK_SET 0
#define SC_ADD_53_00X 0x0000
#define HZ_TO_SC_FREQ(hz) (((hz)-8000000+2000)/4000)
#define SC_MULT_53_45X 0xc000
#define SM_SDISHARE       (1<<10)
#define SM_TESTS          (1<< 5)
#define SCI_STATUS      0x01
#define SCI_HDAT0       0x08 /* VS1063, VS1053, VS1033, VS1003, VS1011 */

#define SM_SDINEW         (1<<11)

/* Note: code SS_VER=2 is used for both VS1002 and VS1011e */
const uint16_t chipNumber[16] = {
  1001, 1011, 1011, 1003, 1053, 1033, 1063, 1103,
  0, 0, 0, 0, 0, 0, 0, 0
};
uint8_t adpcmHeader[60] = {
  'R', 'I', 'F', 'F',
  0xFF, 0xFF, 0xFF, 0xFF,
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',
  0x14, 0, 0, 0,          /* 20 */
  0x11, 0,                /* IMA ADPCM */
  0x1, 0,                 /* chan */
  0x0, 0x0, 0x0, 0x0,     /* sampleRate */
  0x0, 0x0, 0x0, 0x0,     /* byteRate */
  0, 1,                   /* blockAlign */
  4, 0,                   /* bitsPerSample */
  2, 0,                   /* byteExtraData */
  0xf9, 0x1,              /* samplesPerBlock = 505 */
  'f', 'a', 'c', 't',     /* subChunk2Id */
  0x4, 0, 0, 0,           /* subChunk2Size */
  0xFF, 0xFF, 0xFF, 0xFF, /* numOfSamples */
  'd', 'a', 't', 'a',
  0xFF, 0xFF, 0xFF, 0xFF
};

uint8_t pcmHeader[44] = {
  'R', 'I', 'F', 'F',
  0xFF, 0xFF, 0xFF, 0xFF,
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',
  0x10, 0, 0, 0,          /* 16 */
  0x1, 0,                 /* PCM */
  0x1, 0,                 /* chan */
  0x0, 0x0, 0x0, 0x0,     /* sampleRate */
  0x0, 0x0, 0x0, 0x0,     /* byteRate */
  2, 0,                   /* blockAlign */
  0x10, 0,                /* bitsPerSample */
  'd', 'a', 't', 'a',
  0xFF, 0xFF, 0xFF, 0xFF
};

const uint16_t imaFix[] = {
  0x0007, 0x0001, 0x8010, 0x0006, 0x001c, 0x3e12, 0xb817, 0x3e14, /*    0 */
  0xf812, 0x3e01, 0xb811, 0x0007, 0x9717, 0x0020, 0xffd2, 0x0030, /*    8 */
  0x11d1, 0x3111, 0x8024, 0x3704, 0xc024, 0x3b81, 0x8024, 0x3101, /*   10 */
  0x8024, 0x3b81, 0x8024, 0x3f04, 0xc024, 0x2808, 0x4800, 0x36f1, /*   18 */
  0x9811, 0x0007, 0x0001, 0x8028, 0x0006, 0x0002, 0x2a00, 0x040e,
};
enum AudioFormat {
  afUnknown,
  afRiff,
  afOggVorbis,
  afMp1,
  afMp2,
  afMp3,
  afAacMp4,
  afAacAdts,
  afAacAdif,
  afFlac,
  afWma,
  afMidi,
} audioFormat = afUnknown;

/* Following are for VS1053 and VS1063 */
#define SC_MULT_53_10X 0x0000
#define SC_MULT_53_20X 0x2000
#define SC_MULT_53_25X 0x4000
#define SC_MULT_53_30X 0x6000
#define SC_MULT_53_35X 0x8000
#define SC_MULT_53_40X 0xa000
#define SC_MULT_53_45X 0xc000
#define SC_MULT_53_50X 0xe000


/* Following are for VS1053 and VS1063 */
#define SC_ADD_53_00X 0x0000
#define SC_ADD_53_10X 0x0800
#define SC_ADD_53_15X 0x1000
#define SC_ADD_53_20X 0x1800


/* VS1063 / VS1053 Parametric */
#define PAR_CHIP_ID                  0x1e00 /* VS1063, VS1053, 32 bits */
#define PAR_VERSION                  0x1e02 /* VS1063, VS1053 */
#define PAR_CONFIG1                  0x1e03 /* VS1063, VS1053 */
#define PAR_PLAY_SPEED               0x1e04 /* VS1063, VS1053 */
#define PAR_BITRATE_PER_100          0x1e05 /* VS1063 */
#define PAR_BYTERATE                 0x1e05 /* VS1053 */
#define PAR_END_FILL_BYTE            0x1e06 /* VS1063, VS1053 */
#define PAR_RATE_TUNE                0x1e07 /* VS1063,         32 bits */
#define PAR_PLAY_MODE                0x1e09 /* VS1063 */
#define PAR_SAMPLE_COUNTER           0x1e0a /* VS1063,         32 bits */
#define PAR_VU_METER                 0x1e0c /* VS1063 */
#define PAR_AD_MIXER_GAIN            0x1e0d /* VS1063 */
#define PAR_AD_MIXER_CONFIG          0x1e0e /* VS1063 */
#define PAR_PCM_MIXER_RATE           0x1e0f /* VS1063 */
#define PAR_PCM_MIXER_FREE           0x1e10 /* VS1063 */
#define PAR_PCM_MIXER_VOL            0x1e11 /* VS1063 */
#define PAR_EQ5_DUMMY                0x1e12 /* VS1063 */
#define PAR_EQ5_LEVEL1               0x1e13 /* VS1063 */
#define PAR_EQ5_FREQ1                0x1e14 /* VS1063 */
#define PAR_EQ5_LEVEL2               0x1e15 /* VS1063 */
#define PAR_EQ5_FREQ2                0x1e16 /* VS1063 */
#define PAR_JUMP_POINTS              0x1e16 /*         VS1053 */
#define PAR_EQ5_LEVEL3               0x1e17 /* VS1063 */
#define PAR_EQ5_FREQ3                0x1e18 /* VS1063 */
#define PAR_EQ5_LEVEL4               0x1e19 /* VS1063 */
#define PAR_EQ5_FREQ4                0x1e1a /* VS1063 */
#define PAR_EQ5_LEVEL5               0x1e1b /* VS1063 */
#define PAR_EQ5_UPDATED              0x1e1c /* VS1063 */
#define PAR_SPEED_SHIFTER            0x1e1d /* VS1063 */
#define PAR_EARSPEAKER_LEVEL         0x1e1e /* VS1063 */
#define PAR_SDI_FREE                 0x1e1f /* VS1063 */
#define PAR_AUDIO_FILL               0x1e20 /* VS1063 */
#define PAR_RESERVED0                0x1e21 /* VS1063 */
#define PAR_RESERVED1                0x1e22 /* VS1063 */
#define PAR_RESERVED2                0x1e23 /* VS1063 */
#define PAR_RESERVED3                0x1e24 /* VS1063 */
#define PAR_LATEST_SOF               0x1e25 /* VS1063,         32 bits */
#define PAR_LATEST_JUMP              0x1e26 /*         VS1053 */
#define PAR_POSITION_MSEC            0x1e27 /* VS1063, VS1053, 32 bits */
#define PAR_RESYNC                   0x1e29 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE 0x0010 /* VS1063, VS1053 */
#define SCI_VOL         0x0B
#define SCI_WRAMADDR    0x07
#define SCI_WRAM        0x06

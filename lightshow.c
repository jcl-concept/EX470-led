#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>

void pr_help()
{
	printf("e47x_lightshow 0.1 by David Kuder - mss (at) thewaffleiron.net\n\n");
	printf("Usage: lightshow <program number>\n\n");
	printf("0: Scan Down\n");
	printf("1: Scan Up\n");
	printf("2: Scan Bounce (KITT)\n");
	printf("3: Pulse Red\n");
	printf("4: Pulse Blue\n");
	printf("5: Pulse Purple\n");
	printf("6: Pulse Cycle\n");
}

int LEDValue[8];
int LEDDelta[8];

#define FACTOR 32
#define MAXVAL 255

#define ABS(a) ((a<0)?(0-a):a)
#define NABS(a) ((a>0)?(0-a):a)

FILE *led[8];

void setleds() {
	int i;

	for(i=0; i<8; i++) {
		fprintf(led[i], "%i\n", LEDValue[i]/FACTOR);
		fflush(led[i]);
	}
}

#define SCAN_DOWN	0
#define SCAN_UP		1
#define SCAN_BOUNCE	2
#define PULSE_RED	3
#define PULSE_BLUE	4
#define PULSE_PURPLE	5
#define PULSE_CYCLE	6

int main (int argc, char **argv)
{
	int i;
	int TempA = 0, TempB = -1, mode = SCAN_BOUNCE; //SCAN_DOWN;

	for(i=1; i<argc; i++) {
		if((!strcmp(argv[i], "--help") || (!strcmp(argv[i], "-h")) {
			pr_help();
			exit(0);
		}
		mode = atoi(argv[i]);
	}

	for(i=0; i<8; i++) {
		LEDValue[i] = MAXVAL * FACTOR;	LEDDelta[i] = -1;
	}

	switch(mode) {
		case PULSE_BLUE:
			for(i=0; i<4; i++) {
				LEDValue[i] = 0;	LEDDelta[i] = 0;
			}
			for(i=4; i<8; i++) {
				LEDValue[i] = MAXVAL * FACTOR;	LEDDelta[i] = -1;
			}
			break;
		case PULSE_RED:
			for(i=0; i<4; i++) {
				LEDValue[i] = MAXVAL * FACTOR;	LEDDelta[i] = -1;
			}
			for(i=4; i<8; i++) {
				LEDValue[i] = 0;	LEDDelta[i] = 0;
			}
			break;
		case PULSE_PURPLE:
			for(i=0; i<8; i++) {
				LEDValue[i] = MAXVAL * FACTOR;	LEDDelta[i] = -1;
			}
			break;
		case PULSE_CYCLE:
			TempB = 1;
			for(i=0; i<4; i++) {
				LEDValue[i] = 0;	LEDDelta[i] = 1;
			}
			for(i=4; i<8; i++) {
				LEDValue[i] = 0;	LEDDelta[i] = 0;
			}
			break;
	}

	for(i=0; i<4; i++) {
		char filename[256];
		sprintf(filename, "/sys/class/leds/hpex47x:red:hdd%i/brightness", i);
		led[i]=fopen(filename, "w");
		if(led[i]==NULL) {
			fprintf(stderr, "Unable to open %s\r\n", filename);
			exit(-1);
		}
		sprintf(filename, "/sys/class/leds/hpex47x:blue:hdd%i/brightness", i);
		led[i+4]=fopen(filename, "w");
		if(led[i+4]==NULL) {
			fprintf(stderr, "Unable to open %s\r\n", filename);
			exit(-1);
		}
	}

	while(1) {
		switch(mode) {
			case SCAN_UP:
				if(TempA > 11) {
					TempA = 0;
				}
				if(TempA < 8) {
					LEDValue[TempA] += 3;
					if(LEDValue[TempA] > MAXVAL*FACTOR) {
						LEDValue[TempA] = MAXVAL*FACTOR;
						LEDDelta[TempA] = -1;
						TempA++;
					}
				} else {
					LEDValue[TempA & 3] += 3;
					LEDValue[(TempA & 3) + 4] += 3;
					if(LEDValue[TempA & 3] > MAXVAL*FACTOR) {
						LEDValue[TempA & 3] = MAXVAL*FACTOR;
						LEDDelta[TempA & 3] = -1;
						LEDValue[(TempA & 3) + 4] = MAXVAL*FACTOR;
						LEDDelta[(TempA & 3) + 4] = -1;
						TempA++;
					}
				}
				for(i=0; i<8; i++) {
					if(LEDDelta[i] != 0) {
						LEDValue[i]--;
						if(LEDValue[i] < 0) {
							LEDValue[i] = 0;
							LEDDelta[i] = 0;
						}
					}
				}
				break;
			case SCAN_DOWN:
				if(TempA < 0) {
					TempA = 11;
				}
				if(TempA < 8) {
					LEDValue[TempA] += 3;
					if(LEDValue[TempA] > MAXVAL*FACTOR) {
						LEDValue[TempA] = MAXVAL*FACTOR;
						LEDDelta[TempA] = -1;
						TempA--;
					}
				} else {
					LEDValue[TempA & 3] += 3;
					LEDValue[(TempA & 3) + 4] += 3;
					if(LEDValue[TempA & 3] > MAXVAL*FACTOR) {
						LEDValue[TempA & 3] = MAXVAL*FACTOR;
						LEDDelta[TempA & 3] = -1;
						LEDValue[(TempA & 3) + 4] = MAXVAL*FACTOR;
						LEDDelta[(TempA & 3) + 4] = -1;
						TempA--;
					}
				}
				for(i=0; i<8; i++) {
					if(LEDDelta[i] != 0) {
						LEDValue[i]--;
						if(LEDValue[i] < 0) {
							LEDValue[i] = 0;
							LEDDelta[i] = 0;
						}
					}
				}
				break;
			case SCAN_BOUNCE:
				if(TempA > 3) {
					TempA = 3;
					TempB = -1;
				} else if(TempA < 0) {
					TempA = 0;
					TempB = 1;
				}
				LEDValue[TempA] += 3;
				if(LEDValue[TempA] > MAXVAL*FACTOR) {
					LEDValue[TempA] = MAXVAL*FACTOR;
					LEDDelta[TempA] = -1;
					TempA += TempB;
				}
				for(i=0; i<8; i++) {
					if(LEDDelta[i] != 0) {
						LEDValue[i]--;
						if(LEDValue[i] < 0) {
							LEDValue[i] = 0;
							LEDDelta[i] = 0;
						}
					}
				}
				break;
			case PULSE_BLUE:
			case PULSE_RED:
			case PULSE_PURPLE:
				for(i=0; i<8; i++) {
					LEDValue[i] += LEDDelta[i];
					if(LEDValue[i] < 0) { LEDDelta[i] = ABS(LEDDelta[i]); LEDValue[i] = 0; }
					else if(LEDValue[i] > (MAXVAL*FACTOR)) { LEDDelta[i] = NABS(LEDDelta[i]); LEDValue[i] = (MAXVAL*FACTOR); }
				}
				break;
			case PULSE_CYCLE:
				for(i=0; i<8; i++) {
					LEDValue[i] += LEDDelta[i];
				}
				switch(TempA) {
					case 0:
						for(i=0; i<4; i++) {
							LEDValue[i] = 0;
							LEDDelta[i] = 1;
						}
						for(i=4; i<8; i++) {
							LEDValue[i] = 0;
							LEDDelta[i] = 0;
						}
						TempA++;
						break;
					case 1:
						if(LEDValue[0] > MAXVAL*FACTOR) {
							for(i=0; i<4; i++) {
								LEDValue[i] = MAXVAL*FACTOR;
								LEDDelta[i] = -1;
							}
							TempA++;
						}
						break;
					case 2:
						if(LEDValue[0] < 0) {
							for(i=0; i<8; i++) {
								LEDValue[i] = 0;
								LEDDelta[i] = 0;
							}
							TempA++;
						}
						break;
					case 3:
						for(i=0; i<4; i++) {
							LEDValue[i] = 0;
							LEDDelta[i] = 0;
						}
						for(i=4; i<8; i++) {
							LEDValue[i] = 0;
							LEDDelta[i] = 1;
						}
						TempA++;
						break;
					case 4:
						if(LEDValue[4] > MAXVAL*FACTOR) {
							for(i=4; i<8; i++) {
								LEDValue[i] = MAXVAL*FACTOR;
								LEDDelta[i] = -1;
							}
							TempA++;
						}
						break;
					case 5:
						if(LEDValue[4] < 0) {
							for(i=0; i<8; i++) {
								LEDValue[i] = 0;
								LEDDelta[i] = 0;
							}
							TempA++;
						}
						break;
					case 6:
						for(i=0; i<8; i++) {
							LEDValue[i] = 0;
							LEDDelta[i] = 1;
						}
						TempA++;
						break;
					case 7:
						if(LEDValue[0] > MAXVAL*FACTOR) {
							for(i=0; i<8; i++) {
								LEDValue[i] = MAXVAL*FACTOR;
								LEDDelta[i] = -1;
							}
							TempA++;
						}
						break;
					case 8:
						if(LEDValue[0] < 0) {
							for(i=0; i<8; i++) {
								LEDValue[i] = 0;
								LEDDelta[i] = 0;
							}
							TempA=0;
						}
						break;
				}
				break;
		}			

		setleds();
		usleep(1);
	}

out:
	for(i=0; i<8; i++) {
		if(led[i] != NULL)
			fclose(led[i]);
	}

	return(0);
}

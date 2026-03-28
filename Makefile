CC = gcc
CFLAGS := -Wall -s -O2
EZINFO = ezinfo
EZUNFN = ezunfn
EZREFN = ezrefn
EZDEC_715 = ezdec_715
EZENC_715 = ezenc_715
EZDEC_720 = ezdec_720
EZENC_720 = ezenc_720
EZINS = ezins
EZABM = ezabm
EZWSSF = ezwssf
ABM2BMP = abm2bmp

.PHONY: all clean default

default: all

all: $(EZINFO) $(EZUNFN) $(EZREFN) $(EZDEC_715) $(EZENC_715) $(EZDEC_720) $(EZENC_720) $(EZINS) $(EZABM) $(EZWSSF) $(ABM2BMP)

$(EZINFO): ezinfo.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZUNFN): ezunfn.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZREFN): ezunfn.c
	$(CC) $(CFLAGS) -D__REFN $^ $(LDFLAGS) -o $@

$(EZDEC_715): ezdec_715.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZENC_715): ezdec_715.c
	$(CC) $(CFLAGS) -D__ENC $^ $(LDFLAGS) -o $@

$(EZDEC_720): ezdec_720.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZENC_720): ezdec_720.c
	$(CC) $(CFLAGS) -D__ENC $^ $(LDFLAGS) -o $@

$(EZINS): ezins.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZABM): ezabm.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EZWSSF): ezwssf.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(ABM2BMP): abm2bmp.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(EZINFO) $(EZUNFN) $(EZREFN) $(EZDEC_715) $(EZENC_715) $(EZDEC_720) $(EZENC_720) $(EZINS) $(EZABM) $(EZWSSF) $(ABM2BMP)

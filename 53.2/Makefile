SRCMODULES = ../lib/error_functions.c ../lib/get_num.c

.PHONY: build
build: clean cl sv cl_abs_ns sv_abs_ns

-include ../include/main.mk

%: us_xfr_%.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o us_xfr_$@

.PHONY: clean_%
clean_%:
	rm -f us_xfr_$*.o us_xfr_$*

.PHONY: clean
clean: clean_cl clean_sv;

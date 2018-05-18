include config.mk

LIB := ../lib/
CFLAGS += -I../lib
DIST_IMPL := dist_stroke.o dist_assign_strokes.o wmcbm.o
LOOKUP_IMPL := lookup_strokecnt.o $(DIST_IMPL)

all: simpl.elf sdl.elf test_metric.elf bench_db_open.elf dump_distmat.elf\
     test_lookup.elf subset.elf damage.elf makesc.elf assign.elf assign2mcmf.elf assign_gen.elf mcmf.elf

%.o: $(LIB)%.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.elf:
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

sdl.elf: CFLAGS += $(SDL_CFLAGS)
sdl.elf: LDFLAGS += $(SDL_LDFLAGS)

simpl.elf: simpl.o kanji.o std.o svg.o klist_io.o

sdl.elf: sdl.o std.o kanji.o $(DIST_IMPL) $(LOOKUP_IMPL) gfx.o

bench_db_open.elf: bench_db_open.o std.o kanji.o $(LOOKUP_IMPL) $(DIST_IMPL)

dump_distmat.elf: dump_distmat.o std.o kanji.o $(DIST_IMPL)

test_lookup.elf: test_lookup.o std.o kanji.o $(LOOKUP_IMPL) klist_io.o

test_metric.elf: test_metric.o std.o

assign.elf: assign.o wmcbm.o

assign2mcmf.elf: assign2mcmf.o

assign_gen.elf: assign_gen.o std.o

mcmf.elf: mcmf.o std.o std.o

subset.elf: subset.o std.o klist_io.o

damage.elf: damage.o std.o klist_io.o

makesc.elf: makesc.o std.o klist_io.o

#makevp.elf: makevp.o std.o $(DIST_IMPL)

makesc.elf: makesc.o klist_io.o std.o $(DIST_IMPL)

clean:
	rm -f *.o *.elf

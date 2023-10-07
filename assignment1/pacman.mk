EXERCISES += pacman
CLEAN_FILES += pacman

pacman: pacman_EOF.c
	$(CC) -o $@ $<

.PHONY: submit give

submit give: pacman.s
	give dp1092 ass1_pacman pacman.s

.PHONY: test autotest

test autotest: pacman.s
	1092 autotest pacman

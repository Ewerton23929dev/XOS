# =============================
# Configurações principais
# =============================
ISO_DIR   := iso/boot
ISO_OUT   := os.iso
LD_SCRIPT := linker.ld

# =============================
# Inclui dependências automáticas
# =============================
-include $(OBJ_C:.o=.d)

CC    := gcc
NASM  := nasm
LD    := ld

-include config.mk
CFLAGS     += -I./src -ffreestanding -fno-pic -m32 -Wall -c -O4
NASMFLAGS  += -f elf32 -Isrc/program/

# =============================
# Seções de código
# =============================
SECTIONS_C   := src src/kernel src/idt src/mmu src/memory src/timer src/keyboard src/vga src/locks src/mouse src/boot/grub src/boot/bootstrap src/console src/program src/cpu
SECTIONS_ASM := src/boot src/idt src/program

# Encontra todos os arquivos automaticamente
SRC_C   := $(foreach dir,$(SECTIONS_C),$(wildcard $(dir)/*.c))
SRC_ASM := $(foreach dir,$(SECTIONS_ASM),$(wildcard $(dir)/*.asm))

ifdef OTIMIZER # OTIMIZER
	SRC_ASM_FAST := $(foreach dir,$(SECTIONS_C),$(wildcard $(dir)/fast_implement/*.asm))
else
	SRC_ASM_FAST :=
endif

SRC_ASM_TOTAL := $(SRC_ASM) $(SRC_ASM_FAST)
OBJ_C   := $(SRC_C:.c=.o)
OBJ_ASM := $(SRC_ASM_TOTAL:.asm=.o)

TOTAL_C := $(words $(SRC_C))
TOTAL_ASM := $(words $(SRC_ASM_TOTAL))

# CORES
YELLOW := \033[0;33m
GREEN := \033[0;32m
BLUE := \033[0;34m
NC := \033[0m


# =============================
# Targets
# =============================
all: $(ISO_OUT)

# Compila C
%.o: %.c
	@bash -c '\
		for i in $(SRC_C); do \
			if [ "$$i" = "$<" ]; then \
				INDEX=$$(( $$(echo $(SRC_C) | tr " " "\n" | grep -n "$$i" | cut -d: -f1) )); \
				echo -e "$(YELLOW)[CC]$(NC) $$INDEX/$(TOTAL_C) $(GREEN)$<$(NC)"; \
			fi; \
		done; \
	'
	@$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

# Compila ASM
# Compila ASM
%.o: %.asm
	@INDEX=$$(echo $(SRC_ASM_TOTAL) | tr " " "\n" | grep -n "$<" | cut -d: -f1); \
	echo "$(BLUE)[ASM]$(NC) $$INDEX/$(TOTAL_ASM) $(GREEN)$<$(NC)"; \
	$(NASM) $(NASMFLAGS) -f elf32 $< -o $@

# Linka todos os objetos (C primeiro, ASM depois)
kernel.elf: $(OBJ_C) $(OBJ_ASM)
	@mkdir -p $(ISO_DIR)
	@echo "\033[0;33m[LD]\033[0m Linking $@"
	@for f in $(OBJ_C) $(OBJ_ASM); do \
		echo "$(YELLOW)[LD]$(NC) Linked $(GREEN)$$f$(NC)"; \
	done
	@$(LD) -g -T $(LD_SCRIPT) -m elf_i386 $(OBJ_C) $(OBJ_ASM) -o $(ISO_DIR)/kernel.elf
	@echo "Link done!"

# Gera ISO com GRUB
$(ISO_OUT): kernel.elf
	@grub-mkrescue -o $(ISO_OUT) iso/ > /dev/null 2>&1
	@echo "ISO done!"

# =============================
# Limpeza
# =============================
clean:
	@rm -rf $(OBJ_C) $(OBJ_C:.o=.d) $(OBJ_ASM) $(ISO_OUT) $(ISO_DIR)/kernel.elf
reset_config:
	@rm -rf config.mk
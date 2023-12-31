TARGET := mcu_shell
BuildParams := -w -mthumb -mcpu=cortex-m4 -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000
LinkParams := -mthumb -mcpu=cortex-m4 -T $(shell find -name "*.ld") -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler -Wl,-Map=out/$(TARGET).map -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80 -Wl,--start-group -lc -lm -Wl,--end-group
IncludePath := -Ilib/inc/
IncludePath += -Icore/
IncludePath += -Imyos/
VPATH := ./lib/src/
VPATH += ./core/
VPATH += ./user/
VPATH += ./myos/
objs := $(addsuffix .o, $(basename $(notdir $(shell find -name "*.[c|s]"))))

all: $(objs)
	@arm-none-eabi-gcc -o out/$(TARGET).elf out/*.o $(LinkParams)
	@arm-none-eabi-objcopy out/$(TARGET).elf -Oihex $(TARGET).hex

%.o: %.c
	@arm-none-eabi-gcc -c $^ -o ./out/$@ $(BuildParams) $(IncludePath)

%.o: %.s
	@arm-none-eabi-gcc -c $^ -o ./out/$@ $(BuildParams) $(IncludePath)

flash:
	@stm32flash -R -i -dtr,dtr,:-dtr,-rts -w $(TARGET).hex /dev/ttyUSB0

clean:
	@rm out/*
	@rm *.hex

debug:
	@echo objs is $(objs)
	@echo VPATH is $(VPATH)
	@echo CFLAGS is $(CFLAGS)
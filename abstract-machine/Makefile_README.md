html 目标
makefile
复制代码
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html
html: 定义了一个目标 html，它会将当前 Makefile 转换为 HTML 格式。
cat Makefile：读取 Makefile 内容。
sed 's/^\([^#]\)/ \1/g'：用 sed 命令在每行非注释内容前加上四个空格，增加缩进。
markdown_py：使用 markdown_py（Python 的 Markdown 解析工具）将内容转换为 HTML 格式，并输出到 Makefile.html 文件。
.PHONY: html：标记 html 为伪目标，不与同名文件冲突。
1. 基本设置和检查
makefile
复制代码
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif
ifeq ($(MAKECMDGOALS),)：如果没有指定目标（即，运行 make 时没有参数）。
MAKECMDGOALS = image：将 MAKECMDGOALS 设置为 image。
.DEFAULT_GOAL = image：设定默认目标为 image，即运行 make 时会构建 image 目标。
makefile
复制代码
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)：检查 MAKECMDGOALS 中是否包含 clean、clean-all 或 html 目标，如果没有包含则继续执行下方的环境检查。
makefile
复制代码
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])
$(info ...)：输出构建信息，将 $(NAME)、$(MAKECMDGOALS)、$(ARCH) 的值显示出来，提示当前构建目标和架构。
makefile
复制代码
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif
$(wildcard $(AM_HOME)/am/include/am.h)：检查路径 $(AM_HOME)/am/include/ 下是否存在 am.h 文件。
如果不存在，使用 $(error ...) 输出错误提示，表示 AM_HOME 必须指向有效的 Abstract Machine 仓库。
makefile
复制代码
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))：定义 ARCHS 变量，用于存储在 $(AM_HOME)/scripts/ 目录下所有 .mk 文件的基名（即文件名去掉后缀）。
makefile
复制代码
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif
ifeq ($(filter $(ARCHS), $(ARCH)), )：检查 $(ARCH) 是否在支持的架构列表 $(ARCHS) 中。
如果不在，则使用 $(error ...) 输出错误提示，提示支持的架构列表和当前值。
makefile
复制代码
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))
ARCH_SPLIT = $(subst -, ,$(ARCH))：将 $(ARCH) 中的 - 替换为空格，以便分割 ISA 和 PLATFORM。
ISA = $(word 1,$(ARCH_SPLIT))：取 ARCH_SPLIT 的第一个单词，赋值给 ISA（如 x86_64）。
PLATFORM = $(word 2,$(ARCH_SPLIT))：取 ARCH_SPLIT 的第二个单词，赋值给 PLATFORM（如 qemu）。
makefile
复制代码
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif
ifeq ($(flavor SRCS), undefined)：检查 SRCS 变量是否定义。
如果 SRCS 未定义，则输出错误 Nothing to build。
2. 通用编译目标
makefile
复制代码
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))
WORK_DIR = $(shell pwd)：设置工作目录为当前路径。
DST_DIR = $(WORK_DIR)/build/$(ARCH)：构建文件将输出到 build/$(ARCH)。
$(shell mkdir -p $(DST_DIR))：创建目标目录 DST_DIR，若路径不存在则新建。
makefile
复制代码
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a
IMAGE_REL：相对路径的二进制文件名。
IMAGE：使用 abspath 转换 IMAGE_REL 为绝对路径。
ARCHIVE：静态库文件的路径。
makefile
复制代码
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib)
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))
OBJS：生成的目标文件路径。
LIBS：按字母顺序列出需要的库，默认包含 am 和 klib。
LINKAGE：待链接的目标文件和库文件路径，包含 OBJS 和 LIBS。
3. 通用编译标志
makefile
复制代码
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf
设置汇编器、编译器、C++ 编译器、链接器、归档工具和其他工具。CROSS_COMPILE 可以设置工具链前缀。
4. 架构特定配置
makefile
复制代码
-include $(AM_HOME)/scripts/$(ARCH).mk
-include $(AM_HOME)/scripts/$(ARCH).mk：条件包含 $(ARCH).mk，如果文件存在则加载。
5. 编译规则
makefile
复制代码
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)
定义从 .c 文件编译成 .o 文件的规则：
mkdir -p $(dir $@)：创建目标文件所在目录。
$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)：使用 GNU C 语言标准编译 .c 文件。
6. 清理规则
makefile
复制代码
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean
clean：删除 Makefile.html 和构建目录 build/。
编译规则（剩余部分）
makefile
复制代码
### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)
针对 .cc 文件的编译规则：
@mkdir -p $(dir $@)：创建目标文件所在目录。
@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)：使用 C++17 标准编译 .cc 文件，并生成 .o 文件。
makefile
复制代码
### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)
针对 .cpp 文件的编译规则，与 .cc 的规则类似。
makefile
复制代码
### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)
针对汇编 .S 文件的编译规则：
使用汇编器 $(AS) 对 .S 文件进行预处理和汇编，生成 .o 文件。
库依赖的递归构建规则
makefile
复制代码
### Rule (recursive make): build a dependent library (am, klib, ...)
$(LIBS): %:
	@$(MAKE) -s -C $(AM_HOME)/$* archive
为依赖的库（如 am、klib）设置了递归构建规则。
$(MAKE) -s -C $(AM_HOME)/$* archive：在指定的子目录下递归调用 make，目标为 archive。
-C $(AM_HOME)/$*：切换到库的目录，$* 会匹配 $(LIBS) 中的每一个库名。
链接规则：生成最终的 ELF 文件
makefile
复制代码
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif
$(IMAGE).elf 规则：
通过链接器 $(LD) 或 C++ 编译器 $(CXX) 将目标文件和库文件链接为 ELF 文件。
$(filter $(ARCH),native) 判断 ARCH 是否为 native 架构：
如果是，使用 $(CXX) 编译器，且链接时启用 --whole-archive 选项；
否则使用 $(LD) 直接链接。
归档规则：生成静态库 .a
makefile
复制代码
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^
$(ARCHIVE) 规则：
使用归档工具 $(AR) 创建静态库文件 $(ARCHIVE)，包含所有目标文件 $(OBJS)。
包含依赖文件 .d
makefile
复制代码
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))
使用 gcc 编译器的 -MMD 选项生成的 .d 依赖文件：
通过 -include 包含 .d 文件，确保 make 知道源文件的依赖关系。
在文件更新时，make 会自动重新编译依赖文件。
构建顺序控制
makefile
复制代码
image: image-dep
archive: $(ARCHIVE)
image-dep: $(LIBS) $(IMAGE).elf
.NOTPARALLEL: image-dep
.PHONY: image image-dep archive run $(LIBS)
image 依赖 image-dep，构建时会优先构建 image-dep 目标。
archive 依赖 $(ARCHIVE)，构建时会生成静态库文件。
image-dep 依赖于所有库 $(LIBS) 和 $(IMAGE).elf。
.NOTPARALLEL: image-dep：禁止 image-dep 目标的并行构建。
清理规则（剩余部分）
makefile
复制代码
### Clean all sub-projects within depth 2 (and ignore errors)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)
CLEAN_ALL 变量通过 find 命令查找当前目录下的子目录（包含 Makefile 文件）。
clean-all 目标将调用每个子目录的 clean 规则，清理所有子项目文件夹的构建输出。

TOPDIR ?= $(CURDIR)

include $(TOPDIR)/Rules/Setup.mk

TARGET		:=	ImGui_Example
BUILD		:=	Build
SOURCES		:=	Plugin/Source ImGui/Source ImBackend/Source
DATA		:=	
INCLUDES	:=	Plugin/Include ImGui/Include ImBackend/Include

include $(TOPDIR)/Rules/Flags.mk

ifneq ($(BUILD),$(notdir $(CURDIR)))

include $(TOPDIR)/Rules/Prepare.mk

else

include $(TOPDIR)/Rules/Build.mk

endif
##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=basic
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/pablo/Documentos/mybasic/codelite_project
ProjectPath            :=/home/pablo/Documentos/mybasic/codelite_project
IntermediateDirectory  :=build-$(WorkspaceConfiguration)
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Pablo A. Arrobbio
Date                   :=30/10/22
CodeLitePath           :=/home/pablo/.codelite
MakeDirCommand         :=mkdir -p
LinkerName             :=/usr/bin/g++-11
SharedObjectLinkerName :=/usr/bin/g++-11 -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=/home/pablo/Documentos/mybasic/codelite_project/build-$(WorkspaceConfiguration)/bin
OutputFile             :=build-$(WorkspaceConfiguration)/bin/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++-11
CC       := /usr/bin/gcc-11
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_src_core_token.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_error.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_eval.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_let.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_builtin.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_memory.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_oper.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ast.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_basic.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_dim.c$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_debugtools_lvar.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_vars.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_print.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_src_core_token.c$(ObjectSuffix): ../src/core/token.c $(IntermediateDirectory)/up_src_core_token.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/token.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_token.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_token.c$(DependSuffix): ../src/core/token.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_token.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_token.c$(DependSuffix) -MM ../src/core/token.c

$(IntermediateDirectory)/up_src_core_token.c$(PreprocessSuffix): ../src/core/token.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_token.c$(PreprocessSuffix) ../src/core/token.c

$(IntermediateDirectory)/up_src_core_error.c$(ObjectSuffix): ../src/core/error.c $(IntermediateDirectory)/up_src_core_error.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/error.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_error.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_error.c$(DependSuffix): ../src/core/error.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_error.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_error.c$(DependSuffix) -MM ../src/core/error.c

$(IntermediateDirectory)/up_src_core_error.c$(PreprocessSuffix): ../src/core/error.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_error.c$(PreprocessSuffix) ../src/core/error.c

$(IntermediateDirectory)/up_src_core_eval.c$(ObjectSuffix): ../src/core/eval.c $(IntermediateDirectory)/up_src_core_eval.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/eval.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_eval.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_eval.c$(DependSuffix): ../src/core/eval.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_eval.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_eval.c$(DependSuffix) -MM ../src/core/eval.c

$(IntermediateDirectory)/up_src_core_eval.c$(PreprocessSuffix): ../src/core/eval.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_eval.c$(PreprocessSuffix) ../src/core/eval.c

$(IntermediateDirectory)/up_src_core_let.c$(ObjectSuffix): ../src/core/let.c $(IntermediateDirectory)/up_src_core_let.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/let.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_let.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_let.c$(DependSuffix): ../src/core/let.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_let.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_let.c$(DependSuffix) -MM ../src/core/let.c

$(IntermediateDirectory)/up_src_core_let.c$(PreprocessSuffix): ../src/core/let.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_let.c$(PreprocessSuffix) ../src/core/let.c

$(IntermediateDirectory)/up_src_core_builtin.c$(ObjectSuffix): ../src/core/builtin.c $(IntermediateDirectory)/up_src_core_builtin.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/builtin.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_builtin.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_builtin.c$(DependSuffix): ../src/core/builtin.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_builtin.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_builtin.c$(DependSuffix) -MM ../src/core/builtin.c

$(IntermediateDirectory)/up_src_core_builtin.c$(PreprocessSuffix): ../src/core/builtin.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_builtin.c$(PreprocessSuffix) ../src/core/builtin.c

$(IntermediateDirectory)/up_src_core_memory.c$(ObjectSuffix): ../src/core/memory.c $(IntermediateDirectory)/up_src_core_memory.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/memory.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_memory.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_memory.c$(DependSuffix): ../src/core/memory.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_memory.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_memory.c$(DependSuffix) -MM ../src/core/memory.c

$(IntermediateDirectory)/up_src_core_memory.c$(PreprocessSuffix): ../src/core/memory.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_memory.c$(PreprocessSuffix) ../src/core/memory.c

$(IntermediateDirectory)/up_src_core_oper.c$(ObjectSuffix): ../src/core/oper.c $(IntermediateDirectory)/up_src_core_oper.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/oper.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_oper.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_oper.c$(DependSuffix): ../src/core/oper.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_oper.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_oper.c$(DependSuffix) -MM ../src/core/oper.c

$(IntermediateDirectory)/up_src_core_oper.c$(PreprocessSuffix): ../src/core/oper.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_oper.c$(PreprocessSuffix) ../src/core/oper.c

$(IntermediateDirectory)/up_src_core_ast.c$(ObjectSuffix): ../src/core/ast.c $(IntermediateDirectory)/up_src_core_ast.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/ast.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ast.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ast.c$(DependSuffix): ../src/core/ast.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ast.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ast.c$(DependSuffix) -MM ../src/core/ast.c

$(IntermediateDirectory)/up_src_core_ast.c$(PreprocessSuffix): ../src/core/ast.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ast.c$(PreprocessSuffix) ../src/core/ast.c

$(IntermediateDirectory)/up_src_core_basic.c$(ObjectSuffix): ../src/core/basic.c $(IntermediateDirectory)/up_src_core_basic.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/basic.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_basic.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_basic.c$(DependSuffix): ../src/core/basic.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_basic.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_basic.c$(DependSuffix) -MM ../src/core/basic.c

$(IntermediateDirectory)/up_src_core_basic.c$(PreprocessSuffix): ../src/core/basic.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_basic.c$(PreprocessSuffix) ../src/core/basic.c

$(IntermediateDirectory)/up_src_core_dim.c$(ObjectSuffix): ../src/core/dim.c $(IntermediateDirectory)/up_src_core_dim.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/dim.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_dim.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_dim.c$(DependSuffix): ../src/core/dim.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_dim.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_dim.c$(DependSuffix) -MM ../src/core/dim.c

$(IntermediateDirectory)/up_src_core_dim.c$(PreprocessSuffix): ../src/core/dim.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_dim.c$(PreprocessSuffix) ../src/core/dim.c

$(IntermediateDirectory)/up_src_debugtools_lvar.c$(ObjectSuffix): ../src/debugtools/lvar.c $(IntermediateDirectory)/up_src_debugtools_lvar.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/debugtools/lvar.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_debugtools_lvar.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_debugtools_lvar.c$(DependSuffix): ../src/debugtools/lvar.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_debugtools_lvar.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_debugtools_lvar.c$(DependSuffix) -MM ../src/debugtools/lvar.c

$(IntermediateDirectory)/up_src_debugtools_lvar.c$(PreprocessSuffix): ../src/debugtools/lvar.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_debugtools_lvar.c$(PreprocessSuffix) ../src/debugtools/lvar.c

$(IntermediateDirectory)/up_src_core_vars.c$(ObjectSuffix): ../src/core/vars.c $(IntermediateDirectory)/up_src_core_vars.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/vars.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_vars.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_vars.c$(DependSuffix): ../src/core/vars.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_vars.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_vars.c$(DependSuffix) -MM ../src/core/vars.c

$(IntermediateDirectory)/up_src_core_vars.c$(PreprocessSuffix): ../src/core/vars.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_vars.c$(PreprocessSuffix) ../src/core/vars.c

$(IntermediateDirectory)/up_src_core_print.c$(ObjectSuffix): ../src/core/print.c $(IntermediateDirectory)/up_src_core_print.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pablo/Documentos/mybasic/src/core/print.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_print.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_print.c$(DependSuffix): ../src/core/print.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_print.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_print.c$(DependSuffix) -MM ../src/core/print.c

$(IntermediateDirectory)/up_src_core_print.c$(PreprocessSuffix): ../src/core/print.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_print.c$(PreprocessSuffix) ../src/core/print.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)



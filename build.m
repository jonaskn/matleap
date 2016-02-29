% % commented out line 36-38 of PsychIncludes
% % // #if defined(__STDC_UTF_16__) && !defined(CHAR16_T)
% % // typedef __CHAR16_TYPE__ char16_t;
% % // #endif
% check for sdk
if exist('LeapSDK','dir')~=7
    error('The "LeapSDK" folder cannot be found.  A link with this name should point to the directory that contains the SDK.');
end

PTBBasefolder='/Users/jonas/Documents/code/resources/Psychtoolbox-3';
usePTBGetSecs=input('Use Psychtoolbox GetSecs for timestamps? [false]/true\n');
if isempty(usePTBGetSecs)
    usePTBGetSecs = false;
end

if ismac
    libdir='-L./LeapSDK/lib';
    PTBOSName='OSX';
elseif strfind(computer,'PCWIN') 
    PTBOSName='Windows';
	if isempty(strfind('64',mexext))
        libdir='-L.\LeapSDK\lib\x86';
	else
        libdir='-L.\LeapSDK\lib\x64';
	end
elseif strcmp(computer,'GLNX86') || strcmp(computer,'GLNXA64')
    PTBOSName='Linux';
    if isempty(strfind('64',mexext))
        libdir='-L./LeapSDK/lib/x86';
    else
        libdir='-L./LeapSDK/lib/x64';
    end
else
    error('Unknown operating system');
end
 
%1. add common parameters
clear arg;     
j=1;
arg{j}='matleap.cpp';
j=j+1;
arg{j}='-ILeapSDK/include';
j=j+1;
arg{j}='-Imatleap.h';
j=j+1;
arg{j}=libdir;
j=j+1;
arg{j}='-lleap';
j=j+1;

% 2. add Psychtoolbox timing if requested
if usePTBGetSecs
    PTBBasefolder=[PTBBasefolder filesep 'PsychSourceGL' filesep 'Source'];
    args{j}=['' PTBBasefolder filesep PTBOSName filesep 'Base' filesep 'PsychTimeGlue.c'];
    j=j+1;
    args{j}='-DPTBGetSecs=true';
    j=j+1;
    args{j}=['-I' PTBBasefolder filesep 'Common' filesep 'Base'];
    j=j+1;
    args{j}=['-I' PTBBasefolder filesep PTBOSName filesep 'Base'];
    j=j+1;
    args{j}=['-I' PTBBasefolder 'Common' filesep 'GetSecs'];
    j=j+1;
    args{j}='LDFLAGS="\$LDFLAGS -framework CoreAudio"';
    j=j+1;
end

% 2. compile
fprintf('Compiling\n');
fprintf('Evaluating mex')
fprintf(' %s',arg{:});
fprintf('\n');

mex(arg{:})

fprintf('done\n');
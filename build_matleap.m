% % commented out line 36-38 of PsychIncludes
% % // #if defined(__STDC_UTF_16__) && !defined(CHAR16_T)
% % // typedef __CHAR16_TYPE__ char16_t;
% % // #endif
% check for sdk
if exist('LeapSDK','dir')~=7
    error('The "LeapSDK" folder cannot be found.  A link with this name should point to the directory that contains the SDK.');
end

oldpath=cd;
path=mfilename('fullpath');
path=path(1:end-length(mfilename()-1));
cd(path);
cd +matleap

if ~exist('PTB_settings.m','file')
    usePTBGetSecs=input('Use Psychtoolbox GetSecs for timestamps? [false]/true\n');
    if isempty(usePTBGetSecs)
        usePTBGetSecs = false;
    end
    if usePTBGetSecs
        PTBBasefolder=input('please specify the path to a full Psychtoolbox copy\n','s');
    else
        PTBBasefolder=[];
    end
	f=fopen('PTB_settings.m','w');
    fprintf(f,'function [usePTBGetSecs PTBBasefolder]=PTB_settings()\n usePTBGetSecs=%i;\nPTBBasefolder=\''%s\''\n',usePTBGetSecs,PTBBasefolder);
    fclose(f);
    
    fprintf('Saved settings to +matleap.PTB_settings')
end

cd(path)
[usePTBGetSecs, PTBBasefolder]=matleap.PTB_settings();
cd +matleap

if ismac
    libdir='-L../LeapSDK/lib';
    PTBOSName='OSX';
elseif strfind(computer,'PCWIN') 
    PTBOSName='Windows';
	if isempty(strfind('64',mexext))
        libdir='-L..\LeapSDK\lib\x86';
	else
        libdir='-L..\LeapSDK\lib\x64';
	end
elseif strcmp(computer,'GLNX86') || strcmp(computer,'GLNXA64')
    PTBOSName='Linux';
    if isempty(strfind('64',mexext))
        libdir='-L../LeapSDK/lib/x86';
    else
        libdir='-L../LeapSDK/lib/x64';
    end
else
    error('Unknown operating system');
end
 
%1. add common parameters
clear arg;     
j=1;
arg{j}='matleap.cpp';
j=j+1;
arg{j}='-I../LeapSDK/include';
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
    arg{j}=['' PTBBasefolder filesep PTBOSName filesep 'Base' filesep 'PsychTimeGlue.c'];
    j=j+1;
    arg{j}='-DPTBGetSecs=true';
    j=j+1;
    arg{j}=['-I' PTBBasefolder filesep 'Common' filesep 'Base'];
    j=j+1;
    arg{j}=['-I' PTBBasefolder filesep PTBOSName filesep 'Base'];
    j=j+1;
    arg{j}=['-I' PTBBasefolder filesep 'Common' filesep 'GetSecs'];
    j=j+1;
    arg{j}='LDFLAGS="\$LDFLAGS -framework CoreAudio"';
    j=j+1;
end

% 2. compile
fprintf('Compiling\n');
fprintf('Evaluating mex')
fprintf(' %s',arg{:});
fprintf('\n');

mex(arg{:})

fprintf('done\n');

cd(oldpath)
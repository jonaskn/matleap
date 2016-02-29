% @file test_matleap.m
% @brief test matleap functionality
% @author Jeff Perry <jeffsp@gmail.com>
% @version 1.0
% @date 2013-09-12

function test_matleap(print_frames)
    if nargin<1
        print_frames=true;
    end
    % remove matleap mex-file from memory
    clear functions
    % set debug on
    %matleap_debug
    % show version
    [version]=matleap_version;
    fprintf('matleap version %d.%d\n',version(1),version(2));
    % pause to let the hardware wake up
    sleep(1);
    
    % get some frames
    frame_id=-1;
    frames=0;
    fprintf('Testing active retieval of frames\n');
    flist=repmat(matleap_frame, 300, 1);
    tic
    while(toc<1)
        % get a frame
        f=matleap_frame;
        % only count it if it has a different id
        if f.id~=frame_id
            frames=frames+1;
            flist(frames)=f;
            frame_id=f.id;
        end
    end
    s=toc;
    if print_frames
        for iF=1:frames
            print(flist(iF));
        end
    end
    % display performance
    fprintf('\t%d frames\n',frames);
    fprintf('\t%f seconds\n',s);
    fprintf('\t%f fps\n',frames/s);
    
    % listen to frames
    fprintf('\nTesting listening to frames\n');
    matleap(3);
    tic
    sleep(1);
    flist=matleap(2);
    s=toc;
    frames=length(flist);
    if print_frames
        for iF=1:frames
            print(flist(iF));
        end
    end
    % display performance
    fprintf('\t%d frames\n',frames);
    fprintf('\t%f seconds\n',s);
    fprintf('\t%f fps\n',frames/s);
    
    % get some frames while listening
    frame_id=-1;
    frames=0;
    fprintf('\nTesting active retieval of frames while listening\n');
    flist=repmat(matleap_frame, 300, 1);
    tic
    while(toc<1)
        % get a frame
        f=matleap_frame;
        % only count it if it has a different id
        if f.id~=frame_id
            frames=frames+1;
            flist(frames)=f;
            frame_id=f.id;
        end
    end
    s=toc;
    if print_frames
        for iF=1:frames
            print(flist(iF));
        end
    end
    % display performance
    fprintf('\t%d frames\n',frames);
    fprintf('\t%f seconds\n',s);
    fprintf('\t%f fps\n',frames/s);
    

    matleap(4);
end

% sleep for t seconds
function sleep(t)
    tic;
    while (toc<t)
    end
end

% print the contents of a leap frame
function print(f)
    fprintf('frame id %d\n',f.id);
    fprintf('frame timestamp %d\n',f.timestamp);
    fprintf('frame pointables %d\n',length(f.pointables));
    for i=1:length(f.pointables)
        fprintf('pointable %d\n',i);
        fprintf('\tid ');
        fprintf('%d',f.pointables(i).id);
        fprintf('\n');
        fprintf('\tposition ');
        fprintf(' %f',f.pointables(i).position);
        fprintf('\n');
        fprintf('\tvelocity ');
        fprintf(' %f',f.pointables(i).velocity);
        fprintf('\n');
        fprintf('\tdirection ');
        fprintf(' %f',f.pointables(i).direction);
        fprintf('\n');
    end
end

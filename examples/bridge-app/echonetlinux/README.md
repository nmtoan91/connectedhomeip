# Important notes from toanstt

1.  Copy zap files in bridge-common project
2.  In file src/app/clusters/window-covering-server/window-covering-server.cpp

        1. #ifndef EMBER_AF_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT #define
           EMBER_AF_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT (0) #endif
        2. Correct HasFeature() function (bug from the SDK):
           emberAfWindowCoveringClusterUpOrOpenCallback() and
           emberAfWindowCoveringClusterDownOrCloseCallback() if
           (HasFeature(endpoint, Feature::kPositionAwareLift) || true)

            ```cpp
                 if (delegate)
            {
             if (HasFeature(endpoint, Feature::kPositionAwareLift)|| true)
             {
                 LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift,100));
             }

             if (HasFeature(endpoint, Feature::kPositionAwareTilt)|| true)
             {
                 LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt,100));
             }
            }
            ```
            ```cpp

    if (delegate) { if (HasFeature(endpoint, Feature::kPositionAwareLift)||
    true) {
    LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift,0)); }

            if (HasFeature(endpoint, Feature::kPositionAwareTilt)|| true)
            {
                LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt,0));
            }
        }
            ```

3.  In file src/app/clusters/window-covering-server/window-covering-delegate.h
    add int openPercent variable virtual CHIP_ERROR
    HandleMovement(WindowCoveringType type, int openPercent) = 0;
4.  In file src/app/clusters/window-covering-server/window-covering-server.cpp
    ```cpp
    delegate->HandleMovement(WindowCoveringType::Lift,100)
    delegate->HandleMovement(WindowCoveringType::Tilt,100)
    delegate->HandleMovement(WindowCoveringType::Lift,0)
    delegate->HandleMovement(WindowCoveringType::Tilt,0)
    delegate->HandleMovement(WindowCoveringType::Lift,liftValue)
    delegate->HandleMovement(WindowCoveringType::Lift,percent100ths)
    delegate->HandleMovement(WindowCoveringType::Tilt,tiltValue)
    delegate->HandleMovement(WindowCoveringType::Tilt,percent100ths)
    ```







# Manually edit the zap files
## bridge-app.matter
1. Search for "endpoint 0"
2. Search for "endpoint 1"
3. Search for "endpoint 2"
## bridge-app.zap
1. Search for keyword. For example "Window Covering"
2. Put it under "endpointTypes.clusters". Note: COPY TWO OF THEM 



# Commands
| Command | Description |
| --- | --- |
| R | Factory reset |
| E | Safe Exit|
| e | Show mappedechonetLITE endpoints|
| f | Show echonetLITE endpoints and GET/SET|
| g | Show echonetLITE endpoints and GET/SET data|
| h | Show echonetLITE endpoints|
| j | Show mapped addresses|
| j | Show add addresses|
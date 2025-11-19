message(WARNING ">>> DEBUG: Inside app folder, BOARD is set to: '${BOARD}'")

set(NFS_PATH "~/ensc351/public/final")

if(BOARD STREQUAL "1")
   # --- BOARD 1: SPEECH TO TEXT ---
   file(GLOB MY_SOURCES "src/main_stt.c" "src/stt_manager.cpp")
    
   add_executable(board1_stt ${MY_SOURCES})
    
   target_include_directories(board1_stt PRIVATE include)
   target_link_libraries(board1_stt PRIVATE hal whisper stdc++)
    
   # 1. Copy Executable
   add_custom_command(TARGET board1_stt POST_BUILD 
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}"
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:board1_stt>" "${NFS_PATH}/board1_stt"
      COMMENT "Deploying Board 1 Executable to NFS..."
   )

   # 2. Copy Models Directory (Contains Whisper Model)
   add_custom_command(TARGET board1_stt POST_BUILD
     COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}/models"
     COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/models" "${NFS_PATH}/models"
     COMMENT "Deploying Models to NFS..."
   )

elseif(BOARD STREQUAL "2")
   # --- BOARD 2: TRANSLATOR ---
   file(GLOB MY_SOURCES "src/main_trans.c" "src/translator_manager.cpp")
    
   add_executable(board2_trans ${MY_SOURCES})
    
   target_include_directories(board2_trans PRIVATE include)
   target_link_libraries(board2_trans PRIVATE hal llama stdc++)

   # 1. Copy Executable
   add_custom_command(TARGET board2_trans POST_BUILD 
     COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}"
     COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:board2_trans>" "${NFS_PATH}/board2_trans"
     COMMENT "Deploying Board 2 Executable to NFS..."
   )

   # 2. Copy Models Directory (Contains Llama Model)
   add_custom_command(TARGET board2_trans POST_BUILD
     COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}/models"
     COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/models" "${NFS_PATH}/models"
     COMMENT "Deploying Models to NFS..."
   )

elseif(BOARD STREQUAL "3")
   # --- BOARD 3: TTS ---
   file(GLOB MY_SOURCES "src/main_tts.c" "src/tts_manager.cpp")
    
   add_executable(board3_tts ${MY_SOURCES})
    
   target_include_directories(board3_tts PRIVATE include)
   target_link_libraries(board3_tts PRIVATE hal stdc++)

   # 1. Copy Executable
   add_custom_command(TARGET board3_tts POST_BUILD 
     COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}"
     COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:board3_tts>" "${NFS_PATH}/board3_tts"
     COMMENT "Deploying Board 3 Executable to NFS..."
   )

   # 2. Copy Piper Directory (Engine + Voice + Libs)
   # Note: This creates the 'ext/piper' structure on the target
   add_custom_command(TARGET board3_tts POST_BUILD
     COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}/ext/piper"
     COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/ext/piper" "${NFS_PATH}/ext/piper"
     COMMENT "Deploying Piper Engine to NFS..."
   )
elseif(BOARD STREQUAL "4")
    # --- BOARD 4: THE HUB ---
    file(GLOB MY_SOURCES "src/main_hub.c")
    
    add_executable(board4_hub ${MY_SOURCES})
    
    target_include_directories(board4_hub PRIVATE include)
    target_link_libraries(board4_hub PRIVATE hal)

    # Copy Executable to NFS
    add_custom_command(TARGET board4_hub POST_BUILD 
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:board4_hub>" "${NFS_PATH}/board4_hub"
        COMMENT "Deploying Hub Executable to NFS..."
    )

    # Copy Samples Folder
    add_custom_command(TARGET board4_hub POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${NFS_PATH}/samples"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/samples" "${NFS_PATH}/samples"
        COMMENT "Deploying Sample Audio Files..."
    )
endif()


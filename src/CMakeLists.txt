target_sources (${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/Differentiator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/DifferentiatorCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/DifferentiatorOperations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/DifferentiatorIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/DifferentiatorDump.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/OperationsDSL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/TreeOptimizations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/RecursiveDescentParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Differentiator/Lexer.cpp)

# Core poker logic library
add_library(core
    card.cpp
    deck.cpp
    hand_ranking.cpp
    betting_rules.cpp
    hand.cpp
    pot.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PUBLIC common nlohmann_json::nlohmann_json)

# Create models directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/models)
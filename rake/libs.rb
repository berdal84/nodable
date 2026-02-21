require_relative 'base'

# declare here the external libraries we need to build as OBJECTS

#---------------------------------------------------------------------------

$text_editor = new_target_from_base("text_editor", TARGET_TYPE_OBJECTS)
$text_editor.sources |= FileList[
    "libs/ImGuiColorTextEdit/TextEditor.cpp"
]

$whereami = new_target_from_base("whereami", TARGET_TYPE_OBJECTS)
$whereami.sources |= FileList[
    "libs/whereami/src/whereami.c"
]

#---------------------------------------------------------------------------
namespace :libs do

    if PLATFORM_DESKTOP
        task :build => [
            'whereami:build'
        ]
    end

    task :build => [
        'text_editor:build',
    ]

    namespace :text_editor do
        tasks_for_target( $text_editor )
    end

    namespace :whereami do
        tasks_for_target( $whereami )
    end
end
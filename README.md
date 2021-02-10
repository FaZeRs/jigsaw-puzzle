# CPP Project Template
Project template for C++ projects with OpenCV.

## Compiling
>git submodule update --init --recursive  
>mkdir build  
>cd build  
>cmake ..  
>make -j  

## Converting to your own project  
Currently converting this to your own repo requires removing .git folder, initing git again and readding submodule because it would not be referenced if .git is deleted. Then make bitbucket git project and set urls.  

Steps:  
>rm -r ./.git  
>rm -r ./modules/universal-classes/  
>git init  
>git submodule add https://bitbucket.org/giraffe360/universal-classes.git ./modules/universal-classes/  

Open top-level CMakeLists.txt and change ```cpp_project_template``` to your own project name that is going to be used for binary. For example, change ```project(cpp_project_template)``` into ```project(my_project)```.

Make your bitbucket git repo and follow the readme there. Something like:  
>git remote add origin https://bitbucket.org/giraffe360/some_sort_of_repo.git  
>git push -u origin master  

Now everything is set up for your own project.
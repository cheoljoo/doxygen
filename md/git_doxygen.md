# build 
```
git clone https://github.com/doxygen/doxygen.git

cd doxygen
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```



# configure
- If the DOT_CLEANUP tag is set to YES, doxygen will remove the intermediate dot# files that are used to generate the various graphs.# The default value is: YES.# This tag requires that the tag HAVE_DOT is set to YES.DOT_CLEANUP            = YES




# How to pull request
- https://gist.github.com/Chaser324/ce0505fbed06b947d962

```
git remote add upstream https://github.com/doxygen/doxygen.git
git remote -v
git fetch upstream
git branch -va
# git checkout master
git merge upstream/master
````

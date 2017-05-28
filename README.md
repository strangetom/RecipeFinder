# RecipeFinder
Search Recipe Folder.

The are a number of aims of this project.  
* Get some familiarity of GUI programming in C++, so I'm using Qt.  
* Implement a search-as-you-type functionality.   
* Use fuzzy searching to avoid having to have the exact name of the recipe I'm looking for.  
* Gain familiarity with using databases to store and retrieve information.

The fuzzy search is implemented using ```lib_fts``` from [forrestthewoods](https://github.com/forrestthewoods/lib_fts) on Github.

### Installation
1. Download the latest version from the [releases](https://github.com/strangetom/RecipeFinder/releases) page
2. Mark as executable ```chmod +x RecipeFinder```
3. On first run, the database will need creating. From the Options menu, choose _update database_.

Note that the program requires Qt 5.8 to run. 
This program also assumes the following folder structure:
```
.
RecipeFinder
recipes.db
├── Beef/
├── Chicken/
├── Dessert/
├── Images/
├── Lamb/
├── Pork/
├── Seafood/
└── Veggie/
```
The ```Images/``` folder must exist. The recipes reside in the other folders as \*.md files. These folder names can change.

### Demo

![](./recipefinder.gif)



## To do

-   Give user feedback about database update and clean options.
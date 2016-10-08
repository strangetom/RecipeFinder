# RecipeFinder
Search Recipe Folder.

The are a number of aims of this project.  
* Get some familiarity of GUI programming in C++, hence using Qt.  
* Implement a search-as-you-type functionality.   
* Use fuzzy searching to avoid having to have the exact name of the recipe I'm looking for.  

The fuzzy search is implemented using ```lib_fts``` from [forrestthewoods](https://github.com/forrestthewoods/lib_fts) on Github.

### To-do list
1. Use ranked fuzzy matching instead of the basic one currently implemented. Maybe store the match and it's rank in an ordered map?  
2. Application icon and launcher for Ubuntu.  

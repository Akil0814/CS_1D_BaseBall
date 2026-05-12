  
---

### ☐☑Story \# \- Story Basic Description

**Description:**  
Place holder description  
**Pre-regs:**

* Req 1  
* Req 2

**Tasks:**

* Task 1  
* Task 2

**Test Scenario:**

* Test 1  
* Test 2

**Definition of Done:** definition  
**Story Points:** x  
**Assignee: [Erfan Tavassoli](mailto:etavassoli1@ivc.edu)**

---

### ☑Story 1 \- Provide a Sorted list of Teams based on Team Name

**Description:**  
As a baseball fan, I want to be able to see a sorted list of all baseball teams sorted by team name in CLI interface  
**Pre-regs:**

* Database table for baseball teams

**Tasks:**

* Retrieve a list of all baseball teams  
* Sort the list by team names  
* Display the sorted list in the terminal for temporary usage

**Test Scenario:**

* All baseball teams are retrieved  
* Teams are sorted alphabetically by team name

**Definition of Done:** Have a sorted list of all baseball teams displayed in the terminal sorted by team name  
**Story Points:** 1  
**Assignee: [Shahob Shahmirzadi](mailto:sshahmirzadi1@ivc.edu)**  
---

### ☑Story 2 \- Admin Page Login GUI

**Description:**  
As an administrator, I want to be able to login to my admin page with a username and password securely.   
**Pre-regs:**

* Securely stored logins  
* Access to check username/password with stored logins

**Tasks:**

* Create a login page with QT’  
* Have fields for admins to enter username and password  
* Switch pages to the dashboard page(another story) after successful login

**Test Scenario:**

* Attempt login with valid credentials  
  * Switches to another page  
* Attempt login with invalid credentials  
  * Present warning for invalid login

**Definition of Done:** A QT GUI which allows the admin to login in to the admin dashboard with proper credential checking  
**Story Points:** 3  
**Assignee: [Erfan Tavassoli](mailto:etavassoli1@ivc.edu)**  
---

### ☑Story 3 \- View Single Team Information GUI

**Description:**  
As a baseball fan, I want to view detailed information about one baseball team such as their Stadium name, Seating capacity, Location, playing surface, Team name, League, Date opened, Distance to center field, Ballpark typology, Roof type.  
**Pre-regs:**

* Database Tables for all the information  
* Helper functions to retrieve information from database  
* GUI for browsing all teams to be able to select the team to view

**Tasks:**

* Display all the information about the baseball team  
* Be able to go back to the browsing list(another story)

**Test Scenario:**

* All information is present  
* All information is accurate to the baseball team

**Definition of Done:** have a QT page which accurately displays all the information about the baseball team.  
**Story Points: 3**  
**Assignee: @alireza**  
---

### ☑Story 4 \- Display All Teams Sorted by Stadium Name GUI

**Description:**  
As a baseball fan, I want to see all teams sorted by stadium name in a nice clean list.  
**Pre-regs:**

* Database tables for stadium information  
* Sorting methods

**Tasks:**

* Display a list of teams  
* Have the list sorted by stadium name

**Test Scenario:**

* All teams displayed  
* Teams ordered by stadium name

**Definition of Done:** A qt window that   
**Story Points:** 3  
**Assignee: @alireza**  
---

### ☑Story 5 \- Filter by League GUI

**Description:**  
As a baseball fan, I want to filter teams by American or National League so that I can view specific groups in a nice organized list and filter dropdowns.  
**Pre-regs:**

* Database for information  
* Filtering logic  
* Team browsing list GUI

**Tasks:**

* Design a GUI for displaying baseball teams  
* Connect filtering logic to the list

**Test Scenario:**

* All teams are displayed  
* League filter is properly filtering teams

**Definition of Done:** A nice, clean intuitive GUI for being able to filter the displayed baseball teams by league.  
**Story Points:** 3  
**Assignee: @alireza**  
---

### ☐Story 6 \- Sort Stadium Data CLI

**Description:**  
As a baseball fan, I want to view stadium data and be able to sort the information on different stadium characteristics such as capacity, date opened, and topology.  
**Pre-regs:**

* Database

**Tasks:**

* Implement sorting/filtering for stadium information  
* Format information nicely for future GUI story  
* Display information via terminal

**Test Scenario:**

* All stadiums are present  
* filters/sorts accurately apply to data

**Definition of Done:** A terminal interface that will update and show the stadium information with filters/sorting.  
**Story Points:** 2  
**Assignee: [Shahob Shahmirzadi](mailto:sshahmirzadi1@ivc.edu)**  
---

### ☑Story 7 \- Display Team Names with Open Roof GUI

**Description:**  
As a baseball fan, I want to be able to view all baseball teams with an easy way to know if they have an open roof stadium  
**Pre-regs:**

* Database  
* GUI for viewing baseball teams

**Tasks:**

* Implement a filter for stadium roof type  
* Apply filter to existing GUI for team browsing

**Test Scenario:**

* No missing teams  
* Proper filtering for teams with only open roofs

**Definition of Done:** A nice clean GUI updated with new filters for open roof stadiums  
**Story Points:** 3  
**Assignee: [Erfan Tavassoli](mailto:etavassoli1@ivc.edu)**  
---

### ☐Story 10 \- Souvenir Purchasing System CLI

**Description:**  
As a baseball fan, I want to purchase souvenirs at each stadium on a trip to be able to plan my purchases and have it tracked across the app.  
**Pre-regs:**

* Database  
* Trip planning system

**Tasks:**

* Implement backend systems for buying souvenirs  
* Backend system for tracking purchases

**Test Scenario:**

* Purchases accurately tracked to backend with accurate prices

**Definition of Done:** An intuitive backend system that is able to add souvenirs to the purchase history and allow the purchase history to be able to viewed via a CLI interface  
**Story Points:** 5  
**Assignee: @david**  
---

### ☐Story 11 \- Track Total Cost and Items

**Description:**  
As a baseball fan, I want to be able to track the total cost and number of items on my baseball visits to track my spending.  
**Pre-regs:**

* Buying system  
* Stadium traversal system

**Tasks:**

* Track and add information about purchases  
* Store it in the backend  
* Calculate and compile the overall information such as the total cost

**Test Scenario:**

* Desk check of the purchases being accurately added

**Definition of Done:** The purchases done during trips to stadiums are accurately recorded in the backend and displayed temporarily via the terminal.  
**Story Points:** 5  
**Assignee: @david**  
---

### ☐Story 12 \- Admin Dashboard Page Souvenir Changing GUI

**Description:**  
As an admin, I want to be able to manage a stadiums souvenir data by being able to edit existing souvenir information as well as add/remove souvenirs  
**Pre-regs:**

* Database table for stadium and souvenir information  
* Access to the tables to edit and manage informations

**Tasks:**

* Display stadium and per stadium list of souvenirs  
* Have the souvenir list only display items from the selected stadium only  
* Allow editing existing souvenirs  
* Add and remove souvenirs

**Test Scenario:**

* Accurate display of stadiums and the stadium’s souvenir  
* Allow modification of souvenirs(existing, adding, removing) and have changes persist in the backend

**Definition of Done:** A section in the admin dashboard GUI that allows viewing and editing of souvenirs for a stadium.  
**Story Points:** 3  
**Assignee: [Erfan Tavassoli](mailto:etavassoli1@ivc.edu)**
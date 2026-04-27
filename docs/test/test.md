# Testing Record

This document records the basic testing completed for the finished agile stories in the baseball stadium project. The testing approach follows the ideas from the testing slides: mostly manual black-box testing from the user's point of view, with a few simple white-box/manual function checks where it made sense.

## Test Environment

- Application: CS 1D Baseball Stadium project
- Test type: Manual GUI testing and manual function/CLI-style calls
- Data used: MLB stadium and team data included with the project
- Tester focus: Confirm that each completed story can be used and gives reasonable results

## Test Plan ID

CS1D-MLB-TEST-PLAN-001

## Purpose

This document defines the basic testing approach for the completed baseball stadium project stories. It records what was tested, how it was checked, and whether the expected result passed at a simple project level.

## Scope

The current scope covers completed user-facing stories and basic admin login behavior. This includes browsing stadium/team data, sorting, filtering, opening team details, and checking admin login behavior.

Future stories such as trip planning, MST/DFS/BFS, souvenir purchasing, and full maintenance workflows should be tested later when those features are completed.

## Overall Test Strategy

Most testing is manual black-box GUI testing from the user's point of view. The tester opens the application, uses the visible controls, and checks whether the result matches the story requirement.

There are also limited white-box/manual function checks for sorting and data retrieval logic, mainly where CLI-style calls or repository calls make the result easier to verify.

No automated testing framework is currently used for this test record.

## Entry Criteria

- The application builds successfully.
- Required project data is available.
- The story being tested has been implemented.
- The tester can access the needed GUI pages or function calls.

## Exit Criteria

- The main expected behavior passes.
- Any failed test is documented.
- Blocking issues are fixed or recorded.
- The completed story can be reviewed by the team/product owner.

## Roles and Responsibilities

- Developer: Implements the story and fixes bugs.
- Tester/team member: Runs the test cases and records results.
- Product owner/team reviewer: Confirms whether the story meets the requirement.

## Approval Process

A story is considered complete when the expected result passes basic testing. If a test fails, the failure should be documented, the issue should be fixed, and the story should be retested. If the issue is not fixed immediately, it should be recorded as a known issue.

## Configuration Management

GitHub is used for version control. Changes should be committed with meaningful messages so the team can understand what was updated. Testing should be done on the current working branch or on the merged project branch.

## Test Deliverables

- This test plan / testing record.
- Manual test case results.
- Any known issues or failed test notes.

## Glossary

- Black-box testing: Testing the feature from the user's point of view without depending on the source code.
- White-box testing: Testing with some knowledge of the code or internal logic.
- GUI: Graphical user interface.
- Story: A small project requirement written from the user's point of view.
- Expected result: What should happen if the feature works correctly.
- Actual result: What actually happened during testing.

## Features Not Yet Covered

- Full trip planning tests
- MST / DFS / BFS tests
- Souvenir purchase tests
- Admin stadium modification tests
- Add/delete/update souvenir tests
- Persistence testing between application runs
- Invalid input and edge case testing

## Completed Story Tests

### Story 1 - Provide a Sorted List of Teams Based on Team Name

**What was completed:**  
The project can retrieve baseball team information and sort the teams by team name.

**Testing performed:**

- Manually called the team/stadium data retrieval logic.
- Checked that all expected teams were returned from the project data.
- Checked that the team names appeared in alphabetical order.
- Reviewed the output in a simple CLI/manual-call style test to confirm the ordering was readable.

**Result:**  
Passed basic manual testing. The team list was retrieved and sorted by team name as expected.

### Story 2 - Admin Page Login GUI

**What was completed:**  
The admin page has a login screen with username and password fields. A successful login moves the user into the admin dashboard, while an incorrect login shows a warning.

**Testing performed:**

- Opened the admin login page.
- Entered valid admin credentials and confirmed the dashboard page opened.
- Entered the debug login and confirmed it was accepted for development testing.
- Entered incorrect credentials and confirmed a login warning was shown.
- Pressed Enter from the username/password fields to confirm the login action still worked.

**Result:**  
Passed basic manual GUI testing. Valid login paths opened the dashboard, and invalid login attempts showed an error message.

### Story 3 - View Single Team Information GUI

**What was completed:**  
The GUI includes a team/stadium detail view with fields for team name, stadium name, league, location, playing surface, roof type, date opened, seating capacity, center field distance, and ballpark typology.

**Testing performed:**

- Opened the browse window.
- Used the detail button to open the detail window.
- Checked that the detail page contained the expected labels and fields.
- Confirmed the detail window could be closed and the user could return to browsing.

**Result:**  
Passed basic manual GUI testing. The detail page opened correctly and included the required team/stadium information fields.

### Story 4 - Display All Teams Sorted by Stadium Name GUI

**What was completed:**  
The browse GUI includes sorting options, including sorting by stadium name.

**Testing performed:**

- Opened the browse stadium window.
- Selected the stadium name sorting option.
- Checked the displayed stadium list to make sure the order followed stadium names.
- Compared a few visible rows manually to confirm the list changed from team-name style ordering to stadium-name ordering.

**Result:**  
Passed basic manual testing. The browse view supported stadium-name sorting and displayed the list in the expected order.

### Story 5 - Filter by League GUI

**What was completed:**  
The browse GUI includes league filtering so users can view all teams, American League teams, or National League teams.

**Testing performed:**

- Opened the browse stadium window.
- Selected the All Leagues option and confirmed the full list was shown.
- Selected American League and checked that the visible teams belonged to the American League.
- Selected National League and checked that the visible teams belonged to the National League.
- Switched between filters to make sure the table updated each time.

**Result:**  
Passed basic manual GUI testing. The league filter changed the displayed list as expected.

### Story 7 - Display Team Names with Open Roof GUI

**What was completed:**  
The browse GUI includes an open roof filter option so users can view teams with open roof stadiums.

**Testing performed:**

- Opened the browse stadium window.
- Selected the open roof filter option.
- Checked that the displayed teams were limited to stadiums with open roof information.
- Switched away from the filter to make sure the regular list could still be viewed.

**Result:**  
Passed basic manual GUI testing. The open roof filter showed the expected type of stadium group and did not block the normal browsing flow.

## Overall Result

The completed stories passed basic manual testing. The main checks included opening the related GUI pages, using the available filters and sorting controls, trying valid and invalid login cases, and manually calling/reviewing data output where CLI-style testing was appropriate.

No formal automated test framework was added for this test record. The current testing is enough to show that the completed stories were checked at a basic project level, and more detailed automated tests can be added later if needed.

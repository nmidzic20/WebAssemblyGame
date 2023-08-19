const table = document.getElementsByTagName("table")[0];
const pagination = document.getElementById("pagination");
const selectElement = document.getElementById("itemsPerPageSelect");
let selectedItemsPerPage = 10;
let currentPage = 1;
let currentPageButton;
let numRows;
let numPages;

// Desired number of page buttons to be shown at all times, regardless of page numbers they show
// Used only for the case where total page number is greater than number of page buttons
// (AKA page buttons cannot dislay all available page numbers)
let numberOfPageButtons = 7;

selectElement.value = selectedItemsPerPage;
getPagesTotal().then(() => {
  showPage(1);
});
selectElement.addEventListener("change", async () => {
  selectedItemsPerPage = selectElement.value;
  await getPagesTotal();
  showPage(1);
});

async function getPagesTotal() {
  fetch("/api/players")
    .then((response) => response.json())
    .then((data) => {
      numRows = data.length;
      numPages = Math.ceil(numRows / selectedItemsPerPage);
    });
}

function showPage(page) {
  fetch(`/api/players?itemsPerPage=${selectedItemsPerPage}&page=${page}`)
    .then((response) => response.json())
    .then((data) => {
      let tableHtml = "<tr><th>ID</th><th>Username</th><th>Score</th></tr>";

      for (const row of data) {
        tableHtml += `<tr><td>${row.id}</td><td>${row.username}</td><td>${row.score}</td></tr>`;
      }

      tableHtml += "</table>";
      table.innerHTML = tableHtml;

      createPagination(page);
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

function createPagination(page) {
  let links = "";

  if (numPages <= 7)
    for (let i = 1; i <= numPages; i++) {
      links += `<button onclick="showPage(${i})" id=${
        i === page ? "current-page" : ""
      }>${i}</button>`;
    }
  else {
    Array.from({ length: numberOfPageButtons }, (_, index) => index + 1).map(
      (currentButton, index) => {
        let showDotsLeft = false;
        let showDotsRight = false;
        let currPageNumber = page;
        let firstPageNumber = 1;
        let lastPageNumber = numPages;

        // These are the page numbers shown from 2nd to second-to-last button (first and last button always show only first/last page)
        // This array of number places the current (active) page in middle to get the numbers before and after it,
        // so that we get the effect of "moving" with the current page number. The array is adjusted to be within the limitations
        // of the available page numbers
        let pageNumbersToShow = getRangeAroundNumber(currPageNumber, [
          firstPageNumber + 1,
          lastPageNumber - 1,
        ]);

        // The number that a given page button will display (not connected to the button position/index)
        let pageNumber;

        // The first/last button always display only first/last page. Any other button will display page number
        // starting from the first element in the range of pageNumbersToShow, and up (using index for this)
        if (currentButton == 1) pageNumber = firstPageNumber;
        else if (currentButton == numberOfPageButtons)
          pageNumber = lastPageNumber;
        else pageNumber = pageNumbersToShow[0] + index - 1;

        // If the conditions for dots are met, override the page number with the dots. Only second and second-to-last
        // button are candidates for dots, and will show them only if there is more than one available page between
        // the first of the pages the buttons are currently showing and first page number (for left side)
        // Analogously for right side

        // Second button
        if (currentButton == 2 && pageNumbersToShow[0] - firstPageNumber > 1) {
          showDotsLeft = true;
        }

        // Second-to-last button
        if (
          currentButton == numberOfPageButtons - 1 &&
          lastPageNumber - pageNumbersToShow[pageNumbersToShow.length - 1] > 1
        ) {
          showDotsRight = true;
        }

        links +=
          showDotsLeft || showDotsRight
            ? `<button class="dots-button">...</button>`
            : `<button onclick="showPage(${pageNumber})" id=${
                pageNumber === page ? "current-page" : ""
              }>${pageNumber}</button>`;
      }
    );
  }
  pagination.innerHTML = links;
}

function getRangeAroundNumber(number, constraintRange) {
  const start = Math.max(number - 2, constraintRange[0]);
  const end = Math.min(number + 2, constraintRange[1]);
  const result = Array.from(
    { length: end - start + 1 },
    (_, index) => start + index
  );

  let diff = 1;
  while (result.length < 5) {
    if (end == constraintRange[1]) result.unshift(result[0] - diff);
    else if (start == constraintRange[0])
      result.push(result[result.length - 1] + diff);
  }

  return result;
}

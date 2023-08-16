const table = document.getElementsByTagName("table")[0];
const pagination = document.getElementById("pagination");
const selectElement = document.getElementById("itemsPerPageSelect");
let selectedItemsPerPage = 10;
let currentPage = 1;

selectElement.value = selectedItemsPerPage;
createPagination();
showPage(1);

selectElement.addEventListener("change", async () => {
  selectedItemsPerPage = selectElement.value;
  createPagination();
  showPage(1);
});

function createPagination() {
  fetch("/api/players")
    .then((response) => response.json())
    .then((data) => {
      const numRows = data.length;
      const numPages = Math.ceil(numRows / selectedItemsPerPage);
      let links = "";
      for (let i = 1; i <= numPages; i++) {
        links += `<button onclick="showPage(${i})">${i}</button>`;
      }
      pagination.innerHTML = links;
    })
    .catch((error) => {
      console.error("Error:", error);
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

      highlightCurrentPageButton(page);
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

function highlightCurrentPageButton(page) {
  let previousPageButton =
    document.getElementsByTagName("button")[currentPage - 1];
  previousPageButton.id = null;
  console.log("Prev");
  console.log(currentPage);

  currentPage = page;
  console.log("Curr");
  console.log(currentPage);
  let currentPageButton =
    document.getElementsByTagName("button")[currentPage - 1];
  currentPageButton.id = "current-page";
}

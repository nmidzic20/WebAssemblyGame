const table = document.getElementsByTagName("table")[0];
const pagination = document.getElementById("pagination");
const selectElement = document.getElementById("itemsPerPageSelect");
let defaultItemsPerPage = selectElement.value;

createPagination();
showPage(1);

selectElement.addEventListener("change", async () => {
  defaultItemsPerPage = selectElement.value;
  createPagination();
  showPage(1, selectElement.value);
});

function createPagination() {
  const requestData = {
    page: 1,
    itemsPerPage: defaultItemsPerPage,
  };

  fetch("/players", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(requestData),
  })
    .then((response) => response.json())
    .then((data) => {
      const numRows = data.length;
      const numPages = Math.ceil(numRows / defaultItemsPerPage);
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

function showPage(page, itemsPerPage = defaultItemsPerPage) {
  console.log("Show " + page);
  const requestData = {
    page: page,
    itemsPerPage: itemsPerPage,
  };

  fetch("/table", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(requestData),
  })
    .then((response) => response.json())
    .then((data) => {
      let tableHtml = "<tr><th>ID</th><th>Username</th><th>Score</th></tr>";

      for (const row of data) {
        tableHtml += `<tr><td>${row.id}</td><td>${row.username}</td><td>${row.score}</td></tr>`;
      }

      tableHtml += "</table>";
      table.innerHTML = tableHtml;
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

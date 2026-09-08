const $ = (selector) =>
    document.querySelector(selector);

const $$ = (selector) =>
    [...document.querySelectorAll(selector)];


const STORAGE_KEY = "focus_todo_tasks";
const THEME_KEY = "focus_todo_theme";


let tasks =
    JSON.parse(
        localStorage.getItem(STORAGE_KEY) || "[]"
    );

let currentFilter = "all";
let currentView = "all";
let currentPriority = null;

let searchTerm = "";
let newestFirst = true;


/* --------------------------------------------------
   HELPERS
-------------------------------------------------- */

function saveTasks() {

    localStorage.setItem(
        STORAGE_KEY,
        JSON.stringify(tasks)
    );
}


function generateId() {

    if (crypto.randomUUID) {
        return crypto.randomUUID();
    }

    return Date.now() + "-" + Math.random();
}


function today() {

    return new Date()
        .toISOString()
        .slice(0, 10);
}


function isToday(date) {

    return date === today();
}


function isOverdue(date) {

    return date &&
           date < today();
}


function formatDate(date) {

    if (!date) {
        return "";
    }

    return new Date(
        date + "T00:00:00"
    ).toLocaleDateString(
        undefined,
        {
            month: "short",
            day: "numeric"
        }
    );
}


function escapeHTML(value = "") {

    return value.replace(
        /[&<>"']/g,

        character => ({
            "&": "&amp;",
            "<": "&lt;",
            ">": "&gt;",
            '"': "&quot;",
            "'": "&#039;"
        }[character])
    );
}


/* --------------------------------------------------
   SAMPLE DATA
-------------------------------------------------- */

function createDemoTasks() {

    if (tasks.length > 0) {
        return;
    }

    tasks = [

        {
            id: generateId(),

            title: "Plan the week",

            description:
                "Choose the three outcomes that matter most.",

            priority: "high",

            due: today(),

            category: "Personal",

            done: false,

            created: Date.now() - 4000
        },

        {
            id: generateId(),

            title: "Deep work session",

            description:
                "90 minutes of focused work with notifications off.",

            priority: "medium",

            due: today(),

            category: "Work",

            done: false,

            created: Date.now() - 3000
        },

        {
            id: generateId(),

            title: "Read 20 pages",

            description:
                "Continue reading before going to sleep.",

            priority: "low",

            due: today(),

            category: "Learning",

            done: true,

            created: Date.now() - 2000
        }

    ];

    saveTasks();
}


createDemoTasks();


/* --------------------------------------------------
   FILTERING
-------------------------------------------------- */

function getVisibleTasks() {

    let result = [...tasks];


    if (currentFilter === "today") {

        result =
            result.filter(
                task => isToday(task.due)
            );
    }


    if (currentFilter === "upcoming") {

        result =
            result.filter(
                task =>
                    task.due &&
                    task.due > today() &&
                    !task.done
            );
    }


    if (currentFilter === "completed") {

        result =
            result.filter(
                task => task.done
            );
    }


    if (currentPriority) {

        result =
            result.filter(
                task =>
                    task.priority === currentPriority
            );
    }


    if (currentView === "active") {

        result =
            result.filter(
                task => !task.done
            );
    }


    if (currentView === "completed") {

        result =
            result.filter(
                task => task.done
            );
    }


    if (searchTerm) {

        const query =
            searchTerm.toLowerCase();

        result =
            result.filter(task => {

                const content =
                    `${task.title}
                     ${task.description}
                     ${task.category}`;

                return content
                    .toLowerCase()
                    .includes(query);
            });
    }


    result.sort((a, b) => {

        if (newestFirst) {
            return b.created - a.created;
        }

        return (
            (a.due || "9999")
                .localeCompare(
                    b.due || "9999"
                )
        );
    });


    return result;
}


/* --------------------------------------------------
   RENDER
-------------------------------------------------- */

function renderTasks() {

    const list =
        getVisibleTasks();

    const container =
        $("#taskList");


    container.innerHTML =
        list.map(task => {

            const overdue =
                isOverdue(task.due) &&
                !task.done;

            return `

                <article
                    class="task
                    ${task.done ? "done" : ""}
                    ${overdue ? "overdue" : ""}"
                    data-id="${task.id}"
                >

                    <button
                        class="check"
                        aria-label="Complete task"
                    ></button>


                    <div>

                        <div class="task-title">
                            ${escapeHTML(task.title)}
                        </div>

                        ${
                            task.description
                                ? `
                                    <div class="task-description">
                                        ${escapeHTML(task.description)}
                                    </div>
                                  `
                                : ""
                        }


                        <div class="meta">

                            <span
                                class="badge ${task.priority}"
                            >
                                ${
                                    task.priority
                                        .charAt(0)
                                        .toUpperCase() +
                                    task.priority.slice(1)
                                }
                            </span>


                            ${
                                task.category
                                    ? `
                                        <span class="badge">
                                            ${escapeHTML(task.category)}
                                        </span>
                                      `
                                    : ""
                            }


                            ${
                                task.due
                                    ? `
                                        <span class="badge">
                                            ◷ ${formatDate(task.due)}
                                            ${
                                                overdue
                                                    ? " · overdue"
                                                    : ""
                                            }
                                        </span>
                                      `
                                    : ""
                            }

                        </div>

                    </div>


                    <div class="task-actions">

                        <button
                            class="icon-button edit"
                            title="Edit"
                        >
                            ✎
                        </button>

                        <button
                            class="icon-button delete"
                            title="Delete"
                        >
                            ×
                        </button>

                    </div>

                </article>

            `;
        })
        .join("");


    $("#emptyState")
        .classList.toggle(
            "hidden",
            list.length !== 0
        );


    updateStatistics();
}


/* --------------------------------------------------
   STATISTICS
-------------------------------------------------- */

function updateStatistics() {

    const total =
        tasks.length;

    const completed =
        tasks.filter(
            task => task.done
        ).length;

    const todayCount =
        tasks.filter(
            task =>
                isToday(task.due) &&
                !task.done
        ).length;

    const upcoming =
        tasks.filter(
            task =>
                task.due &&
                task.due > today() &&
                !task.done
        ).length;


    $("#allCount").textContent =
        total;

    $("#todayCount").textContent =
        todayCount;

    $("#upcomingCount").textContent =
        upcoming;

    $("#completedCount").textContent =
        completed;


    const percentage =
        total
            ? Math.round(
                completed / total * 100
            )
            : 0;


    $("#progressPercent").textContent =
        percentage + "%";

    $("#completionPercent").textContent =
        percentage + "%";


    $("#progressBar").style.width =
        percentage + "%";


    $("#completionRing")
        .style
        .setProperty(
            "--progress",
            percentage * 3.6 + "deg"
        );


    $("#progressText").textContent =
        `${completed} of ${total} completed`;


    if (percentage === 100 && total > 0) {

        $("#heroTitle").textContent =
            "Everything is done. ✨";

        $("#heroDescription").textContent =
            "Amazing work. Enjoy the progress.";

    } else if (percentage >= 60) {

        $("#heroTitle").textContent =
            "You're on a roll.";

        $("#heroDescription").textContent =
            `${completed} tasks completed so far.`;

    } else {

        $("#heroTitle").textContent =
            "Make today count.";

        $("#heroDescription").textContent =
            "Small steps become big results.";
    }
}


/* --------------------------------------------------
   FILTER BUTTONS
-------------------------------------------------- */

$$(".nav-item")
    .forEach(button => {

        button.addEventListener(
            "click",
            () => {

                currentFilter =
                    button.dataset.filter;

                currentPriority = null;


                $$(".nav-item")
                    .forEach(item =>
                        item.classList.remove(
                            "active"
                        )
                    );


                button.classList.add(
                    "active"
                );


                renderTasks();
            }
        );
    });


$$(".priority-filter")
    .forEach(button => {

        button.addEventListener(
            "click",
            () => {

                const priority =
                    button.dataset.priority;


                if (
                    currentPriority === priority
                ) {

                    currentPriority = null;

                } else {

                    currentPriority = priority;
                }


                renderTasks();
            }
        );
    });


$$(".tab")
    .forEach(button => {

        button.addEventListener(
            "click",
            () => {

                $$(".tab")
                    .forEach(
                        tab =>
                            tab.classList.remove(
                                "active"
                            )
                    );


                button.classList.add(
                    "active"
                );


                currentView =
                    button.dataset.view;


                renderTasks();
            }
        );
    });


/* --------------------------------------------------
   SEARCH
-------------------------------------------------- */

$("#search")
    .addEventListener(
        "input",
        event => {

            searchTerm =
                event.target.value;

            renderTasks();
        }
    );


document.addEventListener(
    "keydown",
    event => {

        if (
            event.key === "/" &&
            document.activeElement.tagName !== "INPUT"
        ) {

            event.preventDefault();

            $("#search").focus();
        }


        if (
            event.key === "n" &&
            document.activeElement.tagName !== "INPUT" &&
            !event.ctrlKey &&
            !event.metaKey
        ) {

            openModal();
        }
    }
);


/* --------------------------------------------------
   SORT
-------------------------------------------------- */

$("#sortButton")
    .addEventListener(
        "click",
        () => {

            newestFirst =
                !newestFirst;


            $("#sortText").textContent =
                newestFirst
                    ? "Newest"
                    : "Due date";


            renderTasks();
        }
    );


/* --------------------------------------------------
   MODAL
-------------------------------------------------- */

function openModal(task = null) {

    $("#taskDialog").showModal();


    $("#modalTitle").textContent =
        task
            ? "Edit task"
            : "Create task";


    $("#taskId").value =
        task?.id || "";


    $("#taskTitle").value =
        task?.title || "";


    $("#taskDescription").value =
        task?.description || "";


    $("#taskPriority").value =
        task?.priority || "medium";


    $("#taskDue").value =
        task?.due || "";


    $("#taskCategory").value =
        task?.category || "";


    setTimeout(
        () => $("#taskTitle").focus(),
        50
    );
}


function closeModal() {

    $("#taskDialog").close();
}


$("#addButton")
    .addEventListener(
        "click",
        () => openModal()
    );


$("#emptyAdd")
    .addEventListener(
        "click",
        () => openModal()
    );


$("#closeModal")
    .addEventListener(
        "click",
        closeModal
    );


$("#cancelModal")
    .addEventListener(
        "click",
        closeModal
    );


$("#taskForm")
    .addEventListener(
        "submit",
        event => {

            event.preventDefault();


            const id =
                $("#taskId").value;


            const data = {

                title:
                    $("#taskTitle")
                        .value
                        .trim(),

                description:
                    $("#taskDescription")
                        .value
                        .trim(),

                priority:
                    $("#taskPriority")
                        .value,

                due:
                    $("#taskDue")
                        .value,

                category:
                    $("#taskCategory")
                        .value
                        .trim()
            };


            if (!data.title) {
                return;
            }


            if (id) {

                const task =
                    tasks.find(
                        item =>
                            item.id === id
                    );


                Object.assign(
                    task,
                    data
                );


                showToast(
                    "Task updated"
                );

            } else {

                tasks.unshift({

                    id: generateId(),

                    ...data,

                    done: false,

                    created: Date.now()
                });


                showToast(
                    "Task created"
                );
            }


            saveTasks();

            closeModal();

            renderTasks();
        }
    );


/* --------------------------------------------------
   TASK ACTIONS
-------------------------------------------------- */

$("#taskList")
    .addEventListener(
        "click",
        event => {

            const card =
                event.target.closest(
                    ".task"
                );


            if (!card) {
                return;
            }


            const id =
                card.dataset.id;


            const task =
                tasks.find(
                    item =>
                        item.id === id
                );


            if (
                event.target.closest(
                    ".check"
                )
            ) {

                task.done =
                    !task.done;


                saveTasks();

                renderTasks();


                showToast(
                    task.done
                        ? "Task completed 🎉"
                        : "Task reopened"
                );
            }


            if (
                event.target.closest(
                    ".edit"
                )
            ) {

                openModal(task);
            }


            if (
                event.target.closest(
                    ".delete"
                )
            ) {

                tasks =
                    tasks.filter(
                        item =>
                            item.id !== id
                    );


                saveTasks();

                renderTasks();

                showToast(
                    "Task deleted"
                );
            }
        }
    );


/* --------------------------------------------------
   THEME
-------------------------------------------------- */

$("#themeButton")
    .addEventListener(
        "click",
        () => {

            document.body
                .classList
                .toggle("dark");


            localStorage.setItem(
                THEME_KEY,
                document.body.classList.contains(
                    "dark"
                )
                    ? "dark"
                    : "light"
            );
        }
    );


if (
    localStorage.getItem(
        THEME_KEY
    ) === "dark"
) {

    document.body
        .classList
        .add("dark");
}


/* --------------------------------------------------
   TOAST
-------------------------------------------------- */

let toastTimer;


function showToast(message) {

    const toast =
        $("#toast");


    toast.textContent =
        message;


    toast.classList.add(
        "show"
    );


    clearTimeout(
        toastTimer
    );


    toastTimer =
        setTimeout(
            () => {

                toast.classList.remove(
                    "show"
                );

            },
            1800
        );
}


/* --------------------------------------------------
   DATE
-------------------------------------------------- */

$("#date").textContent =
    new Date()
        .toLocaleDateString(
            undefined,
            {
                weekday: "long",
                month: "long",
                day: "numeric"
            }
        )
        .toUpperCase();


/* --------------------------------------------------
   INITIAL RENDER
-------------------------------------------------- */

renderTasks();
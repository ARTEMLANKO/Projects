package springmvc.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.*;

import jakarta.validation.Valid;
import springmvc.dao.BookDAO;
import springmvc.dao.PersonDAO;
import springmvc.models.Book;
import springmvc.models.Person;
import springmvc.util.PersonValidator;

import java.util.List;

@Controller
@RequestMapping("/books")
public class BookController {

    private final BookDAO bookDAO;
    private final PersonDAO personDAO;
    private final PersonValidator personValidator;

    @Autowired
    public BookController(BookDAO bookDAO, PersonDAO personDAO, PersonValidator personValidator) {
        this.bookDAO = bookDAO;
        this.personDAO = personDAO;
        this.personValidator = personValidator;
    }

    @PatchMapping("/{id}/assign")
    public String assignBook(@PathVariable("id") int bookId,
                             @RequestParam("personId") int personId) {
        bookDAO.assignBookToPerson(bookId, personId);
        return "redirect:/books/" + bookId;
    }

    @GetMapping("/{id}")
    public String show(@PathVariable("id") int id, Model model) {
        Book book = bookDAO.showBook(id);
        model.addAttribute("book", book);
        boolean hasOwner = bookDAO.hasOwner(id);
        model.addAttribute("hasPeople", hasOwner);
        if (hasOwner) {
            Person owner = bookDAO.getBookOwner(id);
            model.addAttribute("owner", owner);
        } else {
            List<Person> allPeople = personDAO.allPeople();
            model.addAttribute("allPeople", allPeople);
        }
        return "showBook";
    }

    @PatchMapping("/{id}/release")
    public String release(@PathVariable("id") int id) {
        bookDAO.releaseBook(id);
        return "redirect:/books/" + id;
    }

    @GetMapping()
    public String allBooks(Model model) {
        model.addAttribute("books", bookDAO.allBooks());
        return "allBooks";
    }

    @GetMapping("/new")
    public String newBook(@ModelAttribute("book") Book book) {
        return "newBook";
    }

    @PostMapping()
    public String create(@ModelAttribute("book") @Valid Book book,
                         BindingResult bindingResult) {
//        personValidator.validate(person, bindingResult);
        if (bindingResult.hasErrors())
            return "newBook";

        bookDAO.saveBook(book);
        return "redirect:/books";
    }

    @GetMapping("/{id}/edit")
    public String edit(Model model, @PathVariable("id") int id) {
        model.addAttribute("book", bookDAO.showBook(id));
        return "editBook";
    }

    @PatchMapping("/{id}")
    public String update(@ModelAttribute("book") @Valid Book book, BindingResult bindingResult,
                         @PathVariable("id") int id) {
//        personValidator.validate(person, bindingResult);
        if (bindingResult.hasErrors())
            return "books/edit";

        bookDAO.updateBook(id, book);
        return "redirect:/books";
    }

    @DeleteMapping("/{id}")
    public String delete(@PathVariable("id") int id) {
        bookDAO.deleteBook(id);
        return "redirect:/books";
    }
}
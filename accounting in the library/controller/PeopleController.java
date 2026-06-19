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
@RequestMapping("/people")
public class PeopleController {

    private final PersonDAO personDAO;
    private final BookDAO bookDAO;
    private final PersonValidator personValidator;

    @Autowired
    public PeopleController(PersonDAO personDAO, BookDAO bookDAO, PersonValidator personValidator) {
        this.personDAO = personDAO;
        this.personValidator = personValidator;
        this.bookDAO = bookDAO;
    }

    @GetMapping()
    public String indexOfPeople(Model model) {
        model.addAttribute("people", personDAO.allPeople());
        return "allPeople";
    }

    @GetMapping("/{id}")
    public String show(@PathVariable("id") int id, Model model) {
        model.addAttribute("person", personDAO.showPeople(id));
        List<Book> personBooks = bookDAO.getBooksByPersonId(id);
        model.addAttribute("hasBooks", !personBooks.isEmpty());
        model.addAttribute("books", personBooks);
        return "showPeople";
    }

    @GetMapping("/new")
    public String newPerson(@ModelAttribute("person") Person person) {
        return "newPeople";
    }

    @PostMapping()
    public String create(@ModelAttribute("person") @Valid Person person,
                         BindingResult bindingResult) {
        personValidator.validate(person, bindingResult);

        if (bindingResult.hasErrors())
            return "newPeople";

        personDAO.savePeople(person);
        return "redirect:/people";
    }

    @GetMapping("/{id}/edit")
    public String edit(Model model, @PathVariable("id") int id) {
        model.addAttribute("person", personDAO.showPeople(id));
        return "editPeople";
    }

    @PatchMapping("/{id}")
    public String update(@ModelAttribute("person") @Valid Person person, BindingResult bindingResult,
                         @PathVariable("id") int id) {

        personValidator.validate(person, bindingResult);

        if (bindingResult.hasErrors())
            return "editPeople";

        personDAO.updatePeople(id, person);
        return "redirect:/people";
    }

    @DeleteMapping("/{id}")
    public String delete(@PathVariable("id") int id) {
        personDAO.deletePeople(id);
        return "redirect:/people";
    }
}
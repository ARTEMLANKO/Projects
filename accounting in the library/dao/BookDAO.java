package springmvc.dao;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.BeanPropertyRowMapper;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Component;
import springmvc.models.Book;
import springmvc.models.Person;

import java.util.List;

@Component
public class BookDAO {
    private final JdbcTemplate jdbcTemplate;

    @Autowired
    public BookDAO(JdbcTemplate jdbcTemplate) {
        this.jdbcTemplate = jdbcTemplate;
    }

    public List<Book> allBooks() {
        return jdbcTemplate.query("SELECT * FROM Book",
                new BeanPropertyRowMapper<>(Book.class));
    }

    public Book showBook(int id) {
        return jdbcTemplate.query("SELECT * FROM Book WHERE id=?",
                        new Object[]{id},
                        new BeanPropertyRowMapper<>(Book.class))
                .stream().findAny().orElse(null);
    }

    public void saveBook(Book book) {
        jdbcTemplate.update("INSERT INTO Book(title, author, year) VALUES(?, ?, ?)",
                book.getTitle(), book.getAuthor(), book.getYear());
    }

    public void updateBook(int id, Book updatedBook) {
        jdbcTemplate.update("UPDATE Book SET title=?, author=?, year=? WHERE id=?",
                updatedBook.getTitle(), updatedBook.getAuthor(),
                updatedBook.getYear(), id);
    }

    public void deleteBook(int id) {
        jdbcTemplate.update("DELETE FROM Book WHERE id=?", id);
    }

    public void assignBookToPerson(int bookId, int personId) {
        jdbcTemplate.update("UPDATE Book SET person_id=? WHERE id=?",
                personId, bookId);
    }

    public void releaseBook(int bookId) {
        jdbcTemplate.update("UPDATE Book SET person_id=NULL WHERE id=?",
                bookId);
    }

    public boolean hasOwner(int bookId) {
        Integer personId = jdbcTemplate.queryForObject(
                "SELECT person_id FROM Book WHERE id=?",
                new Object[]{bookId},
                Integer.class
        );
        return personId != null;
    }

    public Person getBookOwner(int bookId) {
        Integer personId = jdbcTemplate.queryForObject(
                "SELECT person_id FROM Book WHERE id = ?",
                new Object[]{bookId},
                Integer.class
        );

        if (personId == null) {
            return null;
        }

        return jdbcTemplate.queryForObject(
                "SELECT * FROM Person WHERE id = ?",
                new Object[]{personId},
                new BeanPropertyRowMapper<>(Person.class)
        );
    }

    public List<Book> getBooksByPersonId(int personId) {
        return jdbcTemplate.query("SELECT * FROM Book WHERE person_id=?",
                new Object[]{personId},
                new BeanPropertyRowMapper<>(Book.class));
    }
}
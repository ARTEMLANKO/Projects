package springmvc.models;

import jakarta.validation.constraints.*;

public class Person {
    @Pattern(regexp = "[А-ЯЁ]\\p{L}+ [А-ЯЁ]\\p{L}+ [А-ЯЁ]\\p{L}+", message="Полное имя некорректно")
    String fullName;

    @Min(value = 1900, message = "Год рождения должен быть больше 1900")
    @Max(value = 2014, message = "Год рождения должен быть меньше 2014")
    int yearOfBirth;

    int id;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public Person() {}

    public int getYearOfBirth() {
        return yearOfBirth;
    }

    public String getFullName() {
        return fullName;
    }

    public void setFullName(String fullName) {
        this.fullName = fullName;
    }

    public void setYearOfBirth(int yearOfBirth) {
        this.yearOfBirth = yearOfBirth;
    }
}

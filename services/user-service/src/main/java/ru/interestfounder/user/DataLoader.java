package ru.interestfounder.user;

import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Component;
import org.springframework.transaction.annotation.Transactional;

@Component
public class DataLoader {

    private final DistrictRepository districts;

    public DataLoader(DistrictRepository districts) {
        this.districts = districts;
    }

    @PostConstruct
    @Transactional
    public void loadDistricts() {
        if (districts.count() > 0) {
            return;
        }

        districts.save(District.create("Адмиралтейский"));
        districts.save(District.create("Василеостровский"));
        districts.save(District.create("Выборгский"));
        districts.save(District.create("Калининский"));
        districts.save(District.create("Кировский"));
        districts.save(District.create("Колпинский"));
        districts.save(District.create("Красногвардейский"));
        districts.save(District.create("Красносельский"));
        districts.save(District.create("Кронштадтский"));
        districts.save(District.create("Курортный"));
        districts.save(District.create("Московский"));
        districts.save(District.create("Невский"));
        districts.save(District.create("Петроградский"));
        districts.save(District.create("Петродворцовый"));
        districts.save(District.create("Приморский"));
        districts.save(District.create("Пушкинский"));
        districts.save(District.create("Фрунзенский"));
        districts.save(District.create("Центральный"));
    }
}
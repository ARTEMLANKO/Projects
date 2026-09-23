package ru.interestfounder.user;

import jakarta.transaction.Transactional;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.UUID;

@RestController
@RequestMapping("/internal/v1")
public class UserController {

    private final UserRepository users;
    private final DistrictRepository districts;

    public UserController(UserRepository users, DistrictRepository districts) {
        this.users = users;
        this.districts = districts;
    }

    @PutMapping("/users/{telegramUserId}")
    @Transactional
    public UserDto save(@PathVariable long telegramUserId,
                        @RequestBody UpsertRequest request) {
        var user = users.findByTelegramUserId(telegramUserId)
                .orElseGet(() -> users.save(User.create(
                        telegramUserId,
                        request.username())));
        return UserDto.from(user);
    }

    @GetMapping("/users/by-id/{userId}")
    @Transactional
    public UserDto getByUserId(@PathVariable UUID userId) {
        return users.findById(userId)
                .map(UserDto::from)
                .orElse(null);
    }

    @PutMapping("/users/{telegramUserId}/district")
    @Transactional
    public UserDto selectDistrict(@PathVariable long telegramUserId,
                                  @RequestBody SelectDistrictRequest request) {
        var user = users.findByTelegramUserId(telegramUserId)
                .orElse(null);
        var district = districts.findById(request.districtId())
                .orElse(null);
        user.selectDistrict(district);
        return UserDto.from(user);
    }

    @GetMapping("/districts")
    @Transactional
    public List<DistrictDto> districts() {
        return districts.findAll().stream()
                .map(DistrictDto::from)
                .toList();
    }

    public record UpsertRequest(String username) {
    }

    public record SelectDistrictRequest(UUID districtId) {
    }

    public record UserDto(UUID id, long telegramUserId, String username, DistrictDto district) {
        static UserDto from(User u) {
            return new UserDto(
                    u.getId(),
                    u.getTelegramUserId(),
                    u.getUsername(),
                    u.getDistrict() == null ? null : DistrictDto.from(u.getDistrict()));
        }
    }

    public record DistrictDto(UUID id, String name) {
        static DistrictDto from(District d) {
            return new DistrictDto(d.getId(), d.getNameRu());
        }
    }

}
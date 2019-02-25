// Copyright 2018 Maksim Shestakov. All Rights Reserved.

#include "BlueprintJsonLibrary.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"

typedef TSharedPtr<FJsonObject> FJsonObjectPtr;
typedef TSharedPtr<FJsonValue> FJsonValuePtr;

FBlueprintJsonObject UBlueprintJsonLibrary::JsonMake()
{
	FBlueprintJsonObject Object;
	Object.Object = MakeShareable(new FJsonObject);
	return Object;
}

const FBlueprintJsonObject& UBlueprintJsonLibrary::JsonSetField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, const FBlueprintJsonValue& JsonValue)
{
	if (JsonObject.Object.IsValid() && JsonValue.Value.IsValid())
	{
		JsonObject.Object->SetField(FieldName, JsonValue.Value);
	}
	return JsonObject;
}

bool UBlueprintJsonLibrary::JsonHasField(const FBlueprintJsonObject& JsonObject, const FString& FieldName)
{
	if (JsonObject.Object.IsValid())
	{
		return JsonObject.Object->HasField(FieldName);
	}
	return false;
}

bool UBlueprintJsonLibrary::JsonHasTypedField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, EJsonType Type)
{
	if (JsonObject.Object.IsValid())
	{
		if (JsonObject.Object->HasField(FieldName))
		{
			return JsonObject.Object->GetField<EJson::None>(FieldName)->Type == (EJson)Type;
		}
	}
	return false;
}

const FBlueprintJsonObject& UBlueprintJsonLibrary::JsonRemoveField(const FBlueprintJsonObject& JsonObject, const FString& FieldName)
{
	if (JsonObject.Object.IsValid())
	{
		JsonObject.Object->RemoveField(FieldName);
	}
	return JsonObject;
}

FBlueprintJsonValue UBlueprintJsonLibrary::Conv_JsonObjectToJsonValue(const FBlueprintJsonObject& JsonObject, const FString& FieldName)
{
	FBlueprintJsonValue Value;
	if (JsonObject.Object.IsValid())
	{
		Value.Value = JsonObject.Object->GetField<EJson::None>(FieldName);
	}
	return Value;
}

FString UBlueprintJsonLibrary::Conv_JsonObjectToString(const FBlueprintJsonObject& JsonObject)
{
	FString Result;
	if (JsonObject.Object.IsValid())
	{
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Result, 0);
		FJsonSerializer::Serialize(JsonObject.Object.ToSharedRef(), JsonWriter);
	}
	return Result;
}

FString UBlueprintJsonLibrary::Conv_JsonObjectToPrettyString(const FBlueprintJsonObject& JsonObject)
{
	FString Result;
	if (JsonObject.Object.IsValid())
	{
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Result, 0);
		FJsonSerializer::Serialize(JsonObject.Object.ToSharedRef(), JsonWriter);
	}
	return Result;
}

FBlueprintJsonObject UBlueprintJsonLibrary::Conv_StringToJsonObject(const FString& JsonString)
{
	FBlueprintJsonObject Object;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	FJsonSerializer::Deserialize(Reader, Object.Object);
	return Object;
}

const FBlueprintJsonObject& UBlueprintJsonLibrary::JsonMakeField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, const FBlueprintJsonValue& JsonValue)
{
	if (JsonObject.Object.IsValid() && JsonValue.Value.IsValid())
	{
		JsonObject.Object->SetField(FieldName, JsonValue.Value);
	}
	return JsonObject;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeString(const FString& StringValue)
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueString(StringValue));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeInt(int IntValue)
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueNumber(IntValue));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeFloat(float FloatValue)
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueNumber(FloatValue));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeBool(bool BoolValue)
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueBoolean(BoolValue));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeArray(const TArray<FBlueprintJsonValue>& ArrayValue)
{
	FBlueprintJsonValue Value;
	TArray<FJsonValuePtr> Array;
	for (const FBlueprintJsonValue& V : ArrayValue)
	{
		if (V.Value.IsValid())
		{
			Array.Add(V.Value);
		}
	}
	Value.Value = MakeShareable(new FJsonValueArray(Array));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeObject(const FBlueprintJsonObject& ObjectValue)
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueObject(ObjectValue.Object));
	return Value;
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeNull()
{
	FBlueprintJsonValue Value;
	Value.Value = MakeShareable(new FJsonValueNull());
	return Value;
}

EJsonType UBlueprintJsonLibrary::JsonType(const FBlueprintJsonValue& JsonValue)
{
	if (JsonValue.Value.IsValid())
	{
		return (EJsonType)JsonValue.Value->Type;
	}
	return EJsonType::None;
}

bool UBlueprintJsonLibrary::JsonIsNull(const FBlueprintJsonValue& JsonValue)
{
	if (JsonValue.Value.IsValid())
	{
		return JsonValue.Value->IsNull();
	}
	return true;
}

bool UBlueprintJsonLibrary::EquaEqual_JsonValue(const FBlueprintJsonValue& A, const FBlueprintJsonValue& B)
{
	if (A.Value.IsValid() != B.Value.IsValid())
	{
		return false;
	}

	if (A.Value.IsValid() && B.Value.IsValid())
	{
		if (!FJsonValue::CompareEqual(*A.Value, *B.Value))
		{
			return false;
		}
	}

	return true;
}

bool UBlueprintJsonLibrary::NotEqual_JsonValue(const FBlueprintJsonValue& A, const FBlueprintJsonValue& B)
{
	if (A.Value.IsValid() != B.Value.IsValid())
	{
		return true;
	}

	if (A.Value.IsValid() && B.Value.IsValid())
	{
		if (!FJsonValue::CompareEqual(*A.Value, *B.Value))
		{
			return true;
		}
	}

	return false;
}

FString UBlueprintJsonLibrary::Conv_JsonValueToString(const FBlueprintJsonValue& JsonValue)
{	
	if (JsonValue.Value.IsValid())
	{
		return JsonValue.Value->AsString();
	}
	FString Empty;
	return Empty;
}

int UBlueprintJsonLibrary::Conv_JsonValueToInteger(const FBlueprintJsonValue& JsonValue)
{
	if (JsonValue.Value.IsValid())
	{
		int Result = 0;
		JsonValue.Value->TryGetNumber(Result);
		return Result;
	}
	return 0;
}

float UBlueprintJsonLibrary::Conv_JsonValueToFloat(const FBlueprintJsonValue& JsonValue)
{
	if (JsonValue.Value.IsValid())
	{
		return JsonValue.Value->AsNumber();
	}
	return 0.0f;
}

bool UBlueprintJsonLibrary::Conv_JsonValueToBool(const FBlueprintJsonValue& JsonValue)
{
	if (JsonValue.Value.IsValid())
	{
		return JsonValue.Value->AsBool();
	}
	return false;
}

TArray<FBlueprintJsonValue> UBlueprintJsonLibrary::Conv_JsonValueToArray(const FBlueprintJsonValue& JsonValue)
{
	TArray<FBlueprintJsonValue> Result;

	if (JsonValue.Value.IsValid())
	{
		if (JsonValue.Value->Type == EJson::Array)
		{
			for (const auto& Val : JsonValue.Value->AsArray())
			{
				FBlueprintJsonValue Tmp;
				Tmp.Value = Val;
				Result.Add(Tmp);
			}
		}
	}

	return Result;
}

FBlueprintJsonObject UBlueprintJsonLibrary::Conv_JsonValueToObject(const FBlueprintJsonValue& JsonValue)
{
	FBlueprintJsonObject Object;
	if (JsonValue.Value.IsValid())
	{
		Object.Object = JsonValue.Value->AsObject();
	}
	return Object;
}